// $Id: atcommand.c 148 2004-09-30 14:05:37Z MouseJstr $

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

#define STATE_BLIND 0x10

static char command_symbol = '@'; // first char of the commands (by [Yor])


#define ACMD_FUNC(x) bool atcommand_ ## x (int fd, struct map_session_data &sd, const char* command, const char* message)
ACMD_FUNC(broadcast);
ACMD_FUNC(localbroadcast);
ACMD_FUNC(rura);
ACMD_FUNC(where);
ACMD_FUNC(jumpto);
ACMD_FUNC(jump);
ACMD_FUNC(who);
ACMD_FUNC(who2);
ACMD_FUNC(who3);
ACMD_FUNC(whomap);
ACMD_FUNC(whomap2);
ACMD_FUNC(whomap3);
ACMD_FUNC(whogm); // by Yor
ACMD_FUNC(whozeny); // [Valaris]
ACMD_FUNC(happyhappyjoyjoy); // [Valaris]
ACMD_FUNC(save);
ACMD_FUNC(load);
ACMD_FUNC(speed);
ACMD_FUNC(storage);
ACMD_FUNC(guildstorage);
ACMD_FUNC(option);
ACMD_FUNC(hide);
ACMD_FUNC(jobchange);
ACMD_FUNC(die);
ACMD_FUNC(kill);
ACMD_FUNC(alive);
ACMD_FUNC(kami);
ACMD_FUNC(heal);
ACMD_FUNC(item);
ACMD_FUNC(item2);
ACMD_FUNC(itemreset);
ACMD_FUNC(itemcheck);
ACMD_FUNC(baselevelup);
ACMD_FUNC(joblevelup);
ACMD_FUNC(help);
ACMD_FUNC(gm);
ACMD_FUNC(pvpoff);
ACMD_FUNC(pvpon);
ACMD_FUNC(gvgoff);
ACMD_FUNC(gvgon);
ACMD_FUNC(model);
ACMD_FUNC(go);
ACMD_FUNC(monster);
ACMD_FUNC(monstersmall);
ACMD_FUNC(monsterbig);
ACMD_FUNC(spawn);
ACMD_FUNC(killmonster);
ACMD_FUNC(killmonster2);
ACMD_FUNC(refine);
ACMD_FUNC(produce);
ACMD_FUNC(memo);
ACMD_FUNC(gat);
ACMD_FUNC(packet);
ACMD_FUNC(statuspoint);
ACMD_FUNC(skillpoint);
ACMD_FUNC(zeny);
ACMD_FUNC(param);
ACMD_FUNC(guildlevelup);
ACMD_FUNC(makeegg);
ACMD_FUNC(hatch);
ACMD_FUNC(petfriendly);
ACMD_FUNC(pethungry);
ACMD_FUNC(petrename);
ACMD_FUNC(recall);
ACMD_FUNC(recallall);
ACMD_FUNC(revive);
ACMD_FUNC(night);
ACMD_FUNC(day);
ACMD_FUNC(doom);
ACMD_FUNC(doommap);
ACMD_FUNC(raise);
ACMD_FUNC(raisemap);
ACMD_FUNC(kick);
ACMD_FUNC(kickall);
ACMD_FUNC(allskill);
ACMD_FUNC(questskill);
ACMD_FUNC(lostskill);
ACMD_FUNC(spiritball);
ACMD_FUNC(party);
ACMD_FUNC(guild);
ACMD_FUNC(agitstart);
ACMD_FUNC(agitend);
ACMD_FUNC(reloaditemdb);
ACMD_FUNC(reloadmobdb);
ACMD_FUNC(reloadskilldb);
ACMD_FUNC(reloadscript);
ACMD_FUNC(reloadgmdb); // by Yor
ACMD_FUNC(reloadatcommand);
ACMD_FUNC(reloadbattleconf);
ACMD_FUNC(reloadstatusdb);
ACMD_FUNC(reloadpcdb);
ACMD_FUNC(mapexit);
ACMD_FUNC(idsearch);
ACMD_FUNC(mapinfo);
ACMD_FUNC(dye); //** by fritz
ACMD_FUNC(hair_style); //** by fritz
ACMD_FUNC(hair_color); //** by fritz
ACMD_FUNC(stat_all); //** by fritz
ACMD_FUNC(char_block); // by Yor
ACMD_FUNC(char_ban); // by Yor
ACMD_FUNC(char_unblock); // by Yor
ACMD_FUNC(char_unban); // by Yor
ACMD_FUNC(mount_peco); // by Valaris
ACMD_FUNC(char_mount_peco); // by Yor
ACMD_FUNC(guildspy); // [Syrus22]
ACMD_FUNC(partyspy); // [Syrus22]
ACMD_FUNC(repairall); // [Valaris]
ACMD_FUNC(guildrecall); // by Yor
ACMD_FUNC(partyrecall); // by Yor
ACMD_FUNC(nuke); // [Valaris]
ACMD_FUNC(shownpc);
ACMD_FUNC(hidenpc);
ACMD_FUNC(loadnpc);
ACMD_FUNC(unloadnpc);
ACMD_FUNC(servertime); // by Yor
ACMD_FUNC(chardelitem); // by Yor
ACMD_FUNC(jail); // by Yor
ACMD_FUNC(unjail); // by Yor
ACMD_FUNC(disguise); // [Valaris]
ACMD_FUNC(undisguise); // by Yor
ACMD_FUNC(chardisguise); // Kalaspuff
ACMD_FUNC(charundisguise); // Kalaspuff
ACMD_FUNC(email); // by Yor
ACMD_FUNC(effect);//by Apple
ACMD_FUNC(character_cart_list); // by Yor
ACMD_FUNC(addwarp); // by MouseJstr
ACMD_FUNC(follow); // by MouseJstr
ACMD_FUNC(skillon); // by MouseJstr
ACMD_FUNC(skilloff); // by MouseJstr
ACMD_FUNC(killer); // by MouseJstr
ACMD_FUNC(npcmove); // by MouseJstr
ACMD_FUNC(killable); // by MouseJstr
ACMD_FUNC(charkillable); // by MouseJstr
ACMD_FUNC(dropall); // by MouseJstr
ACMD_FUNC(chardropall); // by MouseJstr
ACMD_FUNC(storeall); // by MouseJstr
ACMD_FUNC(charstoreall); // by MouseJstr
ACMD_FUNC(skillid); // by MouseJstr
ACMD_FUNC(useskill); // by MouseJstr
ACMD_FUNC(summon);
ACMD_FUNC(rain);
ACMD_FUNC(snow);
ACMD_FUNC(sakura);
ACMD_FUNC(clouds);
ACMD_FUNC(clouds2);
ACMD_FUNC(fog);
ACMD_FUNC(fireworks);
ACMD_FUNC(leaves);
ACMD_FUNC(adjgmlvl); // by MouseJstr
ACMD_FUNC(adjcmdlvl); // by MouseJstr
ACMD_FUNC(trade); // by MouseJstr
ACMD_FUNC(send); // by davidsiaw
ACMD_FUNC(setbattleflag); // by MouseJstr
ACMD_FUNC(unmute); // [Valaris]
ACMD_FUNC(clearweather); // Dexity
ACMD_FUNC(uptime); // by MC Cameri
ACMD_FUNC(changesex); // by MC Cameri
ACMD_FUNC(mute); // celest
ACMD_FUNC(refresh); // by MC Cameri
ACMD_FUNC(petid); // by MC Cameri
ACMD_FUNC(identify); // by MC Cameri
ACMD_FUNC(gmotd); // Added by MC Cameri, created by davidsiaw
ACMD_FUNC(misceffect); // by MC Cameri
ACMD_FUNC(mobsearch);
ACMD_FUNC(cleanmap);
ACMD_FUNC(npctalk);
ACMD_FUNC(pettalk);
ACMD_FUNC(users);
ACMD_FUNC(autoloot);  // by Upa-Kun
ACMD_FUNC(checkmail); // [Valaris]
ACMD_FUNC(listmail); // [Valaris]
ACMD_FUNC(listnewmail); // [Valaris]
ACMD_FUNC(readmail); // [Valaris]
ACMD_FUNC(sendmail); // [Valaris]
ACMD_FUNC(sendprioritymail); // [Valaris]
ACMD_FUNC(deletemail); // [Valaris]
//ACMD_FUNC(sound); // [Valaris]
ACMD_FUNC(refreshonline); // [Valaris]


ACMD_FUNC(skilltree); // by MouseJstr

ACMD_FUNC(marry); // by MouseJstr
ACMD_FUNC(divorce); // by MouseJstr

ACMD_FUNC(grind); // by MouseJstr
ACMD_FUNC(grind2); // by MouseJstr

#ifdef DMALLOC
ACMD_FUNC(dmstart); // by MouseJstr
ACMD_FUNC(dmtick); // by MouseJstr
#endif

ACMD_FUNC(jumptoid); // by Dino9021
ACMD_FUNC(jumptoid2); // by Dino9021
ACMD_FUNC(recallid); // by Dino9021
ACMD_FUNC(recallid2); // by Dino9021
ACMD_FUNC(kickid); // by Dino9021
ACMD_FUNC(kickid2); // by Dino9021
ACMD_FUNC(reviveid); // by Dino9021
ACMD_FUNC(reviveid2); // by Dino9021
ACMD_FUNC(killid); // by Dino9021
ACMD_FUNC(killid2); // by Dino9021
ACMD_FUNC(charkillableid); // by Dino9021
ACMD_FUNC(charkillableid2);  // by Dino9021
ACMD_FUNC(sound);
ACMD_FUNC(undisguiseall);
ACMD_FUNC(disguiseall);
ACMD_FUNC(changelook);
ACMD_FUNC(mobinfo);	//by Lupus
ACMD_FUNC(adopt); // by Veider

ACMD_FUNC(version); // by Ancyker

ACMD_FUNC(mutearea); // by MouseJstr
ACMD_FUNC(shuffle); // by MouseJstr
ACMD_FUNC(rates); // by MouseJstr

ACMD_FUNC(battleoption);

ACMD_FUNC(iteminfo); // Lupus
ACMD_FUNC(mapflag); // Lupus
ACMD_FUNC(me); //added by massdriller, code by lordalfa
ACMD_FUNC(fakename); //[Valaris]
ACMD_FUNC(size); //[Valaris]
ACMD_FUNC(showexp);
ACMD_FUNC(showdelay);
ACMD_FUNC(monsterignore); // [Valaris]

/*==========================================
 *AtCommandInfo atcommand_info[]ç\ë¢ëÃÇÃíËã`
 *------------------------------------------
 */

// First char of commands is configured in atcommand_athena.conf. Leave @ in this list for default value.
// to set default level, read atcommand_athena.conf first please.
static AtCommandInfo atcommand_info[] = {
	{ AtCommand_Rura,				"@rura",			40, atcommand_rura },
	{ AtCommand_Warp,				"@warp",			40, atcommand_rura },
	{ AtCommand_Where,				"@where",			 1, atcommand_where },
	{ AtCommand_JumpTo,				"@jumpto",			20, atcommand_jumpto }, // + /shift
	{ AtCommand_JumpTo,				"@warpto",			20, atcommand_jumpto },
	{ AtCommand_JumpTo,				"@goto",			20, atcommand_jumpto },
	{ AtCommand_Jump,				"@jump",			40, atcommand_jump },
	{ AtCommand_Who,				"@who",				20, atcommand_who },
	{ AtCommand_Who,				"@whois",			20, atcommand_who },
	{ AtCommand_Who2,				"@who2",			20, atcommand_who2 },
	{ AtCommand_Who3,				"@who3",			20, atcommand_who3 },
	{ AtCommand_WhoMap,				"@whomap",			20, atcommand_whomap },
	{ AtCommand_WhoMap2,			"@whomap2",			20, atcommand_whomap2 },
	{ AtCommand_WhoMap3,			"@whomap3",			20, atcommand_whomap3 },
	{ AtCommand_WhoGM,				"@whogm",			20, atcommand_whogm }, // by Yor
	{ AtCommand_Save,				"@save",			40, atcommand_save },
	{ AtCommand_Load,				"@return",			40, atcommand_load },
	{ AtCommand_Load,				"@load",			40, atcommand_load },
	{ AtCommand_Speed,				"@speed",			40, atcommand_speed },
	{ AtCommand_Storage,			"@storage",			 1, atcommand_storage },
	{ AtCommand_GuildStorage,		"@gstorage",		50, atcommand_guildstorage },
	{ AtCommand_Option,				"@option",			40, atcommand_option },
	{ AtCommand_Hide,				"@hide",			40, atcommand_hide }, // + /hide
	{ AtCommand_JobChange,			"@jobchange",		40, atcommand_jobchange },
	{ AtCommand_JobChange,			"@job",				40, atcommand_jobchange },
	{ AtCommand_Die,				"@die",				 1, atcommand_die },
	{ AtCommand_Kill,				"@kill",			60, atcommand_kill },
	{ AtCommand_Alive,				"@alive",			60, atcommand_alive },
	{ AtCommand_Kami,				"@kami",			40, atcommand_kami },
	{ AtCommand_KamiB,				"@kamib",			40, atcommand_kami },
	{ AtCommand_Heal,				"@heal",			40, atcommand_heal },
	{ AtCommand_Item,				"@item",			60, atcommand_item },
	{ AtCommand_Item2,				"@item2",			60, atcommand_item2 },
	{ AtCommand_ItemReset,			"@itemreset",		40, atcommand_itemreset },
	{ AtCommand_ItemCheck,			"@itemcheck",		60, atcommand_itemcheck },
	{ AtCommand_BaseLevelUp,		"@lvup",			60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,		"@blevel",			60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,		"@baselvlup",		60, atcommand_baselevelup },
	{ AtCommand_JobLevelUp,			"@jlevel",			60, atcommand_joblevelup },
	{ AtCommand_JobLevelUp,			"@joblvup",			60, atcommand_joblevelup },
	{ AtCommand_JobLevelUp,			"@joblvlup",		60, atcommand_joblevelup },
	{ AtCommand_H,					"@h",				20, atcommand_help },
	{ AtCommand_Help,				"@help",			20, atcommand_help },
	{ AtCommand_GM,					"@gm",				100, atcommand_gm },
	{ AtCommand_PvPOff,				"@pvpoff",			40, atcommand_pvpoff },
	{ AtCommand_PvPOn,				"@pvpon",			40, atcommand_pvpon },
	{ AtCommand_GvGOff,				"@gvgoff",			40, atcommand_gvgoff },
	{ AtCommand_GvGOff,				"@gpvpoff",			40, atcommand_gvgoff },
	{ AtCommand_GvGOn,				"@gvgon",			40, atcommand_gvgon },
	{ AtCommand_GvGOn,				"@gpvpon",			40, atcommand_gvgon },
	{ AtCommand_Model,				"@model",			20, atcommand_model },
	{ AtCommand_Go,					"@go",				10, atcommand_go },
	{ AtCommand_Spawn,				"@monster",			50, atcommand_spawn },
	{ AtCommand_Spawn,				"@spawn",			50, atcommand_spawn },
	{ AtCommand_Monster,			"@monster2",		50, atcommand_monster },
	{ AtCommand_MonsterSmall,		"@monstersmall",	50, atcommand_monstersmall },
	{ AtCommand_MonsterBig,			"@monsterbig",		50, atcommand_monsterbig },
	{ AtCommand_KillMonster,		"@killmonster",		60, atcommand_killmonster },
	{ AtCommand_KillMonster2,		"@killmonster2",	40, atcommand_killmonster2 },
	{ AtCommand_Refine,				"@refine",			60, atcommand_refine },
	{ AtCommand_Produce,			"@produce",         60, atcommand_produce },
	{ AtCommand_Memo,				"@memo",			40, atcommand_memo },
	{ AtCommand_GAT,				"@gat",				99, atcommand_gat }, // debug function
	{ AtCommand_Packet,				"@packet",			99, atcommand_packet }, // debug function
	{ AtCommand_Packet,				"@packetmode",		99, atcommand_packet }, // debug function
	{ AtCommand_StatusPoint,		"@stpoint",			60, atcommand_statuspoint },
	{ AtCommand_SkillPoint,			"@skpoint",			60, atcommand_skillpoint },
	{ AtCommand_Zeny,				"@zeny",			60, atcommand_zeny },
	{ AtCommand_Strength,			"@str",				60, atcommand_param },
	{ AtCommand_Agility,			"@agi",				60, atcommand_param },
	{ AtCommand_Vitality,			"@vit",				60, atcommand_param },
	{ AtCommand_Intelligence,		"@int",				60, atcommand_param },
	{ AtCommand_Dexterity,			"@dex",				60, atcommand_param },
	{ AtCommand_Luck,				"@luk",				60, atcommand_param },
	{ AtCommand_GuildLevelUp,		"@guildlvup",		60, atcommand_guildlevelup },
	{ AtCommand_GuildLevelUp,		"@guildlvlup",		60, atcommand_guildlevelup },
	{ AtCommand_MakeEgg,			"@makeegg",			60, atcommand_makeegg },
	{ AtCommand_Hatch,				"@hatch",			60, atcommand_hatch },
	{ AtCommand_PetFriendly,		"@petfriendly",		40, atcommand_petfriendly },
	{ AtCommand_PetHungry,			"@pethungry",		40, atcommand_pethungry },
	{ AtCommand_PetRename,			"@petrename",		 1, atcommand_petrename },
	{ AtCommand_Recall,				"@recall",			60, atcommand_recall }, // + /recall
	{ AtCommand_Revive,				"@revive",			60, atcommand_revive },
	{ AtCommand_Night,				"@night",			80, atcommand_night },
	{ AtCommand_Day,				"@day",				80, atcommand_day },
	{ AtCommand_Doom,				"@doom",			80, atcommand_doom },
	{ AtCommand_DoomMap,			"@doommap",			80, atcommand_doommap },
	{ AtCommand_Raise,				"@raise",			80, atcommand_raise },
	{ AtCommand_RaiseMap,			"@raisemap",		80, atcommand_raisemap },
	{ AtCommand_Kick,				"@kick",			20, atcommand_kick }, // + right click menu for GM "(name) force to quit"
	{ AtCommand_KickAll,			"@kickall",			99, atcommand_kickall },
	{ AtCommand_AllSkill,			"@allskill",		60, atcommand_allskill },
	{ AtCommand_AllSkill,			"@allskills",		60, atcommand_allskill },
	{ AtCommand_AllSkill,			"@skillall",		60, atcommand_allskill },
	{ AtCommand_AllSkill,			"@skillsall",		60, atcommand_allskill },
	{ AtCommand_QuestSkill,			"@questskill",		40, atcommand_questskill },
	{ AtCommand_LostSkill,			"@lostskill",		40, atcommand_lostskill },
	{ AtCommand_SpiritBall,			"@spiritball",		40, atcommand_spiritball },
	{ AtCommand_Party,				"@party",			 1, atcommand_party },
	{ AtCommand_Guild,				"@guild",			50, atcommand_guild },
	{ AtCommand_AgitStart,			"@agitstart",		60, atcommand_agitstart },
	{ AtCommand_AgitEnd,			"@agitend",			60, atcommand_agitend },
	{ AtCommand_MapExit,			"@mapexit",			99, atcommand_mapexit },
	{ AtCommand_IDSearch,			"@idsearch",		60, atcommand_idsearch },
	{ AtCommand_MapMove,			"@mapmove",			40, atcommand_rura }, // /mm command
	{ AtCommand_Broadcast,			"@broadcast",		40, atcommand_broadcast }, // /b and /nb command
	{ AtCommand_LocalBroadcast,		"@localbroadcast",	40, atcommand_localbroadcast }, // /lb and /nlb command
	{ AtCommand_RecallAll,			"@recallall",		80, atcommand_recallall },
	{ AtCommand_ReloadItemDB,		"@reloaditemdb",	99, atcommand_reloaditemdb }, // admin command
	{ AtCommand_ReloadMobDB,		"@reloadmobdb",		99, atcommand_reloadmobdb }, // admin command
	{ AtCommand_ReloadSkillDB,		"@reloadskilldb",	99, atcommand_reloadskilldb }, // admin command
	{ AtCommand_ReloadScript,		"@reloadscript",	99, atcommand_reloadscript }, // admin command
	{ AtCommand_ReloadGMDB,			"@reloadgmdb",		99, atcommand_reloadgmdb }, // admin command
	{ AtCommand_ReloadAtcommand,	"@reloadatcommand",	99, atcommand_reloadatcommand },
	{ AtCommand_ReloadBattleConf,	"@reloadbattleconf",99, atcommand_reloadbattleconf },
	{ AtCommand_ReloadStatusDB,		"@reloadstatusdb",	99, atcommand_reloadstatusdb },
	{ AtCommand_ReloadPcDB,			"@reloadpcdb",		99, atcommand_reloadpcdb },
	{ AtCommand_MapInfo,			"@mapinfo",			99, atcommand_mapinfo },
	{ AtCommand_Dye,				"@dye",				40, atcommand_dye }, // by fritz
	{ AtCommand_Dye,				"@ccolor",			40, atcommand_dye }, // by fritz
	{ AtCommand_Hstyle,				"@hairstyle", 		40, atcommand_hair_style }, // by fritz
	{ AtCommand_Hstyle,				"@hstyle",			40, atcommand_hair_style }, // by fritz
	{ AtCommand_Hcolor,				"@haircolor",		40, atcommand_hair_color }, // by fritz
	{ AtCommand_Hcolor,				"@hcolor",			40, atcommand_hair_color }, // by fritz
	{ AtCommand_StatAll,			"@statall",			60, atcommand_stat_all }, // by fritz
	{ AtCommand_StatAll,			"@statsall",		60, atcommand_stat_all },
	{ AtCommand_StatAll,			"@allstats",		60, atcommand_stat_all }, // by fritz
	{ AtCommand_StatAll,			"@allstat",			60, atcommand_stat_all }, // by fritz
	{ AtCommand_CharBlock,			"@block",			60, atcommand_char_block }, // by Yor
	{ AtCommand_CharBlock,			"@charblock",		60, atcommand_char_block }, // by Yor
	{ AtCommand_CharBan,			"@ban",				60, atcommand_char_ban }, // by Yor
	{ AtCommand_CharBan,			"@banish",			60, atcommand_char_ban }, // by Yor
	{ AtCommand_CharBan,			"@charban",			60, atcommand_char_ban }, // by Yor
	{ AtCommand_CharBan,			"@charbanish",		60, atcommand_char_ban }, // by Yor
	{ AtCommand_CharUnBlock,		"@unblock",			60, atcommand_char_unblock }, // by Yor
	{ AtCommand_CharUnBlock,		"@charunblock",		60, atcommand_char_unblock }, // by Yor
	{ AtCommand_CharUnBan,			"@unban",			60, atcommand_char_unban }, // by Yor
	{ AtCommand_CharUnBan,			"@unbanish",		60, atcommand_char_unban }, // by Yor
	{ AtCommand_CharUnBan,			"@charunban",		60, atcommand_char_unban }, // by Yor
	{ AtCommand_CharUnBan,			"@charunbanish",	60, atcommand_char_unban }, // by Yor
	{ AtCommand_MountPeco,			"@mountpeco",		20, atcommand_mount_peco }, // by Valaris
	{ AtCommand_CharMountPeco,		"@charmountpeco",	50, atcommand_char_mount_peco }, // by Yor
	{ AtCommand_GuildSpy,			"@guildspy",		60, atcommand_guildspy }, // [Syrus22]
	{ AtCommand_PartySpy,			"@partyspy",		60, atcommand_partyspy }, // [Syrus22]
	{ AtCommand_RepairAll,			"@repairall",		60, atcommand_repairall }, // [Valaris]
	{ AtCommand_GuildRecall,		"@guildrecall",		60, atcommand_guildrecall }, // by Yor
	{ AtCommand_PartyRecall,		"@partyrecall",		60, atcommand_partyrecall }, // by Yor
	{ AtCommand_Nuke,				"@nuke",			60, atcommand_nuke }, // [Valaris]
	{ AtCommand_Shownpc,			"@shownpc",			80, atcommand_shownpc }, // []
	{ AtCommand_Hidenpc,			"@hidenpc",			80, atcommand_hidenpc }, // []
	{ AtCommand_Loadnpc,			"@loadnpc",			80, atcommand_loadnpc }, // []
	{ AtCommand_Unloadnpc,			"@unloadnpc",		80, atcommand_unloadnpc }, // []
//	{ AtCommand_Enablenpc,			"@enablenpc",		80, atcommand_enablenpc }, // []
//	{ AtCommand_Disablenpc,			"@disablenpc",		80, atcommand_disablenpc }, // []
	{ AtCommand_ServerTime,			"@time",			 0, atcommand_servertime }, // by Yor
	{ AtCommand_ServerTime,			"@date",			 0, atcommand_servertime }, // by Yor
	{ AtCommand_ServerTime,			"@server_date",		 0, atcommand_servertime }, // by Yor
	{ AtCommand_ServerTime,			"@serverdate",		 0, atcommand_servertime }, // by Yor
	{ AtCommand_ServerTime,			"@server_time",		 0, atcommand_servertime }, // by Yor
	{ AtCommand_ServerTime,			"@servertime",		 0, atcommand_servertime }, // by Yor
	{ AtCommand_CharDelItem,		"@chardelitem",		60, atcommand_chardelitem }, // by Yor
	{ AtCommand_Jail,				"@jail",			60, atcommand_jail }, // by Yor
	{ AtCommand_UnJail,				"@unjail",			60, atcommand_unjail }, // by Yor
	{ AtCommand_UnJail,				"@discharge",		60, atcommand_unjail }, // by Yor
	{ AtCommand_Disguise,			"@disguise",		20, atcommand_disguise }, // [Valaris]
	{ AtCommand_UnDisguise,			"@undisguise",		20, atcommand_undisguise }, // by Yor
	{ AtCommand_CharDisguise,		"@chardisguise",	60, atcommand_chardisguise }, // Kalaspuff
	{ AtCommand_CharUnDisguise,		"@charundisguise",	60, atcommand_charundisguise }, // Kalaspuff
	{ AtCommand_EMail,				"@email",			 0, atcommand_email }, // by Yor
	{ AtCommand_Effect,				"@effect",			40, atcommand_effect }, // by Apple
//	{ AtCommand_Char_Item_List,		"@charitemlist",	40, atcommand_character_item_list }, // by Yor, now #itemlist
//	{ AtCommand_Char_Storage_List,	"@charstoragelist",	40, atcommand_character_storage_list }, // by Yor, now #storagelist
	{ AtCommand_Char_Cart_List,		"@charcartlist",	40, atcommand_character_cart_list }, // by Yor
	{ AtCommand_Follow,				"@follow",			10, atcommand_follow }, // by MouseJstr
	{ AtCommand_AddWarp,			"@addwarp",			20, atcommand_addwarp }, // by MouseJstr
	{ AtCommand_SkillOn,			"@skillon",			20, atcommand_skillon }, // by MouseJstr
	{ AtCommand_SkillOff,			"@skilloff",		20, atcommand_skilloff }, // by MouseJstr
	{ AtCommand_Killer,				"@killer",			60, atcommand_killer }, // by MouseJstr
	{ AtCommand_NpcMove,			"@npcmove",			20, atcommand_npcmove }, // by MouseJstr
	{ AtCommand_Killable,			"@killable",		40, atcommand_killable }, // by MouseJstr
	{ AtCommand_CharKillable,		"@charkillable",	40, atcommand_charkillable }, // by MouseJstr
	{ AtCommand_Dropall,			"@dropall",			40, atcommand_dropall }, // MouseJstr
	{ AtCommand_Chardropall,		"@chardropall",		40, atcommand_chardropall }, // MouseJstr
	{ AtCommand_Storeall,			"@storeall",		40, atcommand_storeall }, // MouseJstr
	{ AtCommand_Charstoreall,		"@charstoreall",	40, atcommand_charstoreall }, // MouseJstr
	{ AtCommand_Skillid,			"@skillid",			40, atcommand_skillid }, // MouseJstr
	{ AtCommand_Useskill,			"@useskill",		40, atcommand_useskill }, // MouseJstr
	{ AtCommand_Rain,				"@rain",			99, atcommand_rain },
	{ AtCommand_Snow,				"@snow",			99, atcommand_snow },
	{ AtCommand_Sakura,				"@sakura",			99, atcommand_sakura },
	{ AtCommand_Clouds,				"@clouds",			99,	atcommand_clouds },
	{ AtCommand_Clouds2,			"@clouds2",			99,	atcommand_clouds2 },
	{ AtCommand_Fog,				"@fog",				99,	atcommand_fog },
	{ AtCommand_Fireworks,			"@fireworks",		99,	atcommand_fireworks },
	{ AtCommand_Leaves,				"@leaves",			99, atcommand_leaves },
	{ AtCommand_Summon,				"@summon",			60, atcommand_summon },
	{ AtCommand_AdjGmLvl,			"@adjgmlvl",		99, atcommand_adjgmlvl },
	{ AtCommand_AdjCmdLvl,			"@adjcmdlvl",		99, atcommand_adjcmdlvl },
	{ AtCommand_Trade,				"@trade",			60, atcommand_trade },
	{ AtCommand_Send,				"@send",			60, atcommand_send },
	{ AtCommand_SetBattleFlag,		"@setbattleflag",	60, atcommand_setbattleflag },
	{ AtCommand_SetBattleFlag,		"@battleoption",	60, atcommand_setbattleflag }, // MouseJstr
	{ AtCommand_UnMute,				"@unmute",			60, atcommand_unmute }, // [Valaris]
	{ AtCommand_Clearweather,		"@clearweather",	99, atcommand_clearweather }, // Dexity
	{ AtCommand_UpTime,				"@uptime",			 0, atcommand_uptime }, // by MC Cameri
//	{ AtCommand_ChangeSex,			"@changesex",		 1, atcommand_changesex }, // by MC Cameri
	{ AtCommand_Mute,				"@mute",			99, atcommand_mute }, // [celest]
	{ AtCommand_Mute,				"@red",				99, atcommand_mute }, // [celest]
	{ AtCommand_WhoZeny,			"@whozeny",			20, atcommand_whozeny }, // [Valaris]
	{ AtCommand_HappyHappyJoyJoy,	"@happyhappyjoyjoy",40, atcommand_happyhappyjoyjoy }, // [Valaris]
	{ AtCommand_Refresh,	        "@refresh",			 0, atcommand_refresh }, // by MC Cameri
	{ AtCommand_PetId,	    	    "@petid",			40, atcommand_petid }, // by MC Cameri
	{ AtCommand_Identify,	   	    "@identify",		40, atcommand_identify }, // by MC Cameri
	{ AtCommand_Gmotd,				"@gmotd",			 0, atcommand_gmotd }, // Added by MC Cameri, created by davidsiaw
	{ AtCommand_MiscEffect,			"@misceffect",		50, atcommand_misceffect }, // by MC Cameri
	{ AtCommand_MobSearch,			"@mobsearch",		 0, atcommand_mobsearch },
	{ AtCommand_CleanMap,			"@cleanmap",		 0, atcommand_cleanmap },
	{ AtCommand_NpcTalk,			"@npctalk",			 0,	atcommand_npctalk },
	{ AtCommand_PetTalk,			"@pettalk",			 0,	atcommand_pettalk },
	{ AtCommand_Users,				"@users",			 0, atcommand_users },
	{ AtCommand_ResetState,			"/reset",			40,	NULL },

	{ AtCommand_CheckMail,			"@checkmail",		 1, atcommand_checkmail }, // [Valaris]
	{ AtCommand_ListMail,			"@listmail",		 1, atcommand_listmail }, // [Valaris]
	{ AtCommand_ListNewMail,		"@listnewmail",		 1, atcommand_listnewmail }, // [Valaris]
	{ AtCommand_ReadMail,			"@readmail",		 1, atcommand_readmail }, // [Valaris]
	{ AtCommand_DeleteMail,			"@deletemail",		 1, atcommand_deletemail }, // [Valaris]
	{ AtCommand_SendMail,			"@sendmail",		 1, atcommand_sendmail }, // [Valaris]
	{ AtCommand_SendPriorityMail,	"@sendprioritymail",80, atcommand_sendmail }, // [Valaris]
	{ AtCommand_RefreshOnline,		"@refreshonline",	99, atcommand_refreshonline }, // [Valaris]

	{ AtCommand_SkillTree,			"@skilltree",		40, atcommand_skilltree }, // [MouseJstr]
	{ AtCommand_Marry,				"@marry",			40, atcommand_marry }, // [MouseJstr]
	{ AtCommand_Divorce,			"@divorce",			40, atcommand_divorce }, // [MouseJstr]
	{ AtCommand_Grind,				"@grind",			99, atcommand_grind }, // [MouseJstr]
	{ AtCommand_Grind2,				"@grind2",			99, atcommand_grind2 }, // [MouseJstr]

#ifdef DMALLOC
	{ AtCommand_DMStart,			"@dmstart",			99, atcommand_dmstart }, // [MouseJstr]
	{ AtCommand_DMTick,				"@dmtick",			99, atcommand_dmtick }, // [MouseJstr]
#endif

	{ AtCommand_JumpToId,			"@jumptoid",		20, atcommand_jumptoid }, // [Dino9021]
	{ AtCommand_JumpToId,			"@warptoid",		20, atcommand_jumptoid }, // [Dino9021]
	{ AtCommand_JumpToId,			"@gotoid",			20, atcommand_jumptoid }, // [Dino9021]
	{ AtCommand_JumpToId2,			"@jumptoid2",		20, atcommand_jumptoid2 }, // [Dino9021]
	{ AtCommand_JumpToId2,			"@warptoid2",		20, atcommand_jumptoid2 }, // [Dino9021]
	{ AtCommand_JumpToId2,			"@gotoid2",			20, atcommand_jumptoid2 }, // [Dino9021]
	{ AtCommand_RecallId,			"@recallid",		60, atcommand_recallid }, // [Dino9021]
	{ AtCommand_RecallId2,			"@recallid2",		60, atcommand_recallid2 }, // [Dino9021]
	{ AtCommand_KickId,				"@kickid",			99, atcommand_kickid }, // [Dino9021]
	{ AtCommand_KickId2,			"@kickid2",			99, atcommand_kickid2 }, // [Dino9021]
	{ AtCommand_ReviveId,			"@reviveid",		60, atcommand_reviveid }, // [Dino9021]
	{ AtCommand_ReviveId2,			"@reviveid2",		60, atcommand_reviveid2 }, // [Dino9021]
	{ AtCommand_KillId,				"@killid",			60, atcommand_killid }, // [Dino9021]
	{ AtCommand_KillId2,			"@killid2",			60, atcommand_killid2 }, // [Dino9021]
	{ AtCommand_CharKillableId,		"@charkillableid",	40, atcommand_charkillableid }, // [Dino9021]
	{ AtCommand_CharKillableId2,	"@charkillableid2",	40, atcommand_charkillableid2 }, // [Dino9021]
	{ AtCommand_Sound,				"@sound",			40,	atcommand_sound },
	{ AtCommand_UndisguiseAll,		"@undisguiseall",	99,	atcommand_undisguiseall },
	{ AtCommand_DisguiseAll,		"@disguiseall",		99,	atcommand_disguiseall },
	{ AtCommand_ChangeLook,			"@changelook",		99,	atcommand_changelook },
	{ AtCommand_AutoLoot,			"@autoloot",		10,	atcommand_autoloot }, // Upa-Kun
	{ AtCommand_MobInfo,			"@mobinfo",			1,	atcommand_mobinfo }, // [Lupus]
	{ AtCommand_MobInfo,			"@monsterinfo",		1,	atcommand_mobinfo }, // [Lupus]
	{ AtCommand_MobInfo,			"@mi",				1,	atcommand_mobinfo }, // [Lupus]
	{ AtCommand_Adopt,              "@adopt",			40, atcommand_adopt }, // [Veider]
	{ AtCommand_Version,			"@version",			0,	atcommand_version },

	{ AtCommand_MuteArea,			"@mutearea",		99, atcommand_mutearea }, // MouseJstr
	{ AtCommand_MuteArea,			"@stfu",			99, atcommand_mutearea }, // MouseJstr
	{ AtCommand_Shuffle,			"@shuffle",			40, atcommand_shuffle }, // MouseJstr
	{ AtCommand_Rates,				"@rates",			10, atcommand_rates }, // MouseJstr
	{ AtCommand_ItemInfo,			"@iteminfo",		1, atcommand_iteminfo }, // [Lupus]
	{ AtCommand_ItemInfo,			"@ii",				1, atcommand_iteminfo }, // [Lupus]
	{ AtCommand_MapFlag,			"@mapflag",			99, atcommand_mapflag }, // [Lupus]

	{ AtCommand_Me,					"@me",				20, atcommand_me }, //added by massdriller, code by lordalfa
	{ AtCommand_FakeName,			"@fakename",		20, atcommand_fakename },
	{ AtCommand_Size,				"@size",			20, atcommand_size },
	{ AtCommand_MonsterIgnore,		"@monsterignore",	99,	atcommand_monsterignore }, // [Valaris]

// add new commands before this line
	{ AtCommand_Unknown,			NULL,				1,	NULL }
};

/*====================================================
 * This function return the name of the job (by [Yor])
 *----------------------------------------------------
 */
const char * job_name(int class_)
{
	switch (class_) {
	case 0:    return "Novice";
	case 1:    return "Swordsman";
	case 2:    return "Mage";
	case 3:    return "Archer";
	case 4:    return "Acolyte";
	case 5:    return "Merchant";
	case 6:    return "Thief";
	case 7:    return "Knight";
	case 8:    return "Priest";
	case 9:    return "Wizard";
	case 10:   return "Blacksmith";
	case 11:   return "Hunter";
	case 12:   return "Assassin";
	case 13:   return "Peco Knight";
	case 14:   return "Crusader";
	case 15:   return "Monk";
	case 16:   return "Sage";
	case 17:   return "Rogue";
	case 18:   return "Alchemist";
	case 19:   return "Bard";
	case 20:   return "Dancer";
	case 21:   return "Peco Crusader";
	case 22:   return "Wedding";
	case 23:   return "Super Novice";
	case 4001: return "Novice High";
	case 4002: return "Swordsman High";
	case 4003: return "Mage High";
	case 4004: return "Archer High";
	case 4005: return "Acolyte High";
	case 4006: return "Merchant High";
	case 4007: return "Thief High";
	case 4008: return "Lord Knight";
	case 4009: return "High Priest";
	case 4010: return "High Wizard";
	case 4011: return "Whitesmith";
	case 4012: return "Sniper";
	case 4013: return "Assassin Cross";
	case 4014: return "Peko Knight";
	case 4015: return "Paladin";
	case 4016: return "Champion";
	case 4017: return "Professor";
	case 4018: return "Stalker";
	case 4019: return "Creator";
	case 4020: return "Clown";
	case 4021: return "Gypsy";
	case 4022: return "Peko Paladin";
	case 4023: return "Baby Novice";
	case 4024: return "Baby Swordsman";
	case 4025: return "Baby Mage";
	case 4026: return "Baby Archer";
	case 4027: return "Baby Acolyte";
	case 4028: return "Baby Merchant";
	case 4029: return "Baby Thief";
	case 4030: return "Baby Knight";
	case 4031: return "Baby Priest";
	case 4032: return "Baby Wizard";
	case 4033: return "Baby Blacksmith";
	case 4034: return "Baby Hunter";
	case 4035: return "Baby Assassin";
	case 4036: return "Baby Peco Knight";
	case 4037: return "Baby Crusader";
	case 4038: return "Baby Monk";
	case 4039: return "Baby Sage";
	case 4040: return "Baby Rogue";
	case 4041: return "Baby Alchemist";
	case 4042: return "Baby Bard";
	case 4043: return "Baby Dancer";
	case 4044: return "Baby Peco Crusader";
	case 4045: return "Super Baby";
	case 4046: return "Taekwon";
	case 4047: return "Star Gladiator";
	case 4048: return "Flying Star Gladiator";
	case 4049: return "Soul Linker";
	}
	return "Unknown Job";
}



// compare function for sorting high to lowest
int hightolow_compare (const void * a, const void * b)
{
  return ( *(uint32*)b - *(uint32*)a );
}

// compare function for sorting lowest to highest
int lowtohigh_compare (const void * a, const void * b)
{
  return ( *(uint32*)a - *(uint32*)b );
}



char *msg_table[MAX_MSG]; // Server messages (0-499 reserved for GM commands, 500-999 reserved for others)

//-----------------------------------------------------------
// Return the message string of the specified number by [Yor]
//-----------------------------------------------------------
const char *msg_txt(size_t msg_number)
{
	if( msg_number < MAX_MSG &&
		msg_table[msg_number][0] != '\0')
		return msg_table[msg_number];
	return "??";
}
/*==========================================
 * Read Message Data
 *------------------------------------------
 */
bool msg_config_read(const char *cfgName)
{
	size_t msg_number;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	static bool initialized = false;

	if((fp = basics::safefopen(cfgName, "r")) == NULL) {
		ShowError("Messages file not found: %s\n", cfgName);
		return false;
	}

	if( !initialized )
	{
		memset(&msg_table[0], 0, sizeof(msg_table[0]) * MAX_MSG);
		initialized=true;
	}
	while(fgets(line, sizeof(line), fp))
	{
		if( !prepare_line(line) )
			continue;
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2)
		{
			if(strcasecmp(w1, "import") == 0)
			{
				msg_config_read(w2);
			}
			else
			{
				msg_number = atoi(w1);
				if(msg_number < MAX_MSG)
				{
					if (msg_table[msg_number] != NULL)
						delete[] msg_table[msg_number];
					msg_table[msg_number] = new char[(1+strlen(w2))];
					memcpy(msg_table[msg_number],w2,1+strlen(w2));
				//	ShowMessage("message #%d: '%s'.\n", msg_number, msg_table[msg_number]);
				}
			}
		}
	}
	fclose(fp);
	return true;
}

/*==========================================
 * Cleanup Message Data
 *------------------------------------------
 */
void do_final_msg ()
{
	size_t i;
	for (i = 0; i < MAX_MSG; ++i)
	{
		if(msg_table[i])
		{
			delete[] msg_table[i];
			msg_table[i] = NULL;
		}
	}
	return;
}


/*==========================================
 * get_atcommand_level @ÉRÉ}ÉìÉhÇÃïKóvÉåÉxÉãÇéÊìæ
 *------------------------------------------
 */
unsigned char get_atcommand_level(const AtCommandType type)
{
	size_t i;
	for (i = 0; atcommand_info[i].type != AtCommand_Unknown; ++i)
		if (atcommand_info[i].type == type)
			return atcommand_info[i].level;
	return 100; // 100: command can not be used
}
/*==========================================
 *
 *------------------------------------------
 */
AtCommandInfo* get_atcommandinfo_byname(const char* name)
{
	size_t i;
	for (i = 0; atcommand_info[i].type != AtCommand_Unknown; ++i)
		if( strcasecmp(atcommand_info[i].command+1, name) == 0 )
			return &atcommand_info[i];
	return NULL;
}

/*==========================================
 *
 *------------------------------------------
 */
bool atcommand_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	AtCommandInfo* p;
	FILE* fp;

	if((fp = basics::safefopen(cfgName, "r")) == NULL) {
		ShowError("At commands configuration file not found: %s\n", cfgName);
		return false;
	}

	while (fgets(line, sizeof(line), fp)) {
		if( !prepare_line(line) )
			continue;

		if (sscanf(line, "%1023[^:]:%1023s", w1, w2) != 2)
			continue;

		if(strcasecmp(w1, "import") == 0)
			atcommand_config_read(w2);
		else if(strcasecmp(w1, "command_symbol") == 0 && w2[0] > 31 &&
				w2[0] != '/' && // symbol of standard ragnarok GM commands
				w2[0] != '%' && // symbol of party chat speaking
				w2[0] != '$' && // symbol of guild chat
				w2[0] != '#')	// symbol of charcommand
			command_symbol = w2[0];
		else
		{
			p = get_atcommandinfo_byname(w1);
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

AtCommandType atcommand(AtCommandInfo& info, const char* message, struct map_session_data &sd, unsigned char level)
{
	if (config.atc_gmonly != 0 && !level) // level = sd.isGM()
		return AtCommand_None;
	if (!message || !*message)
	{
		ShowError("at command message is empty\n");
		return AtCommand_None;
	}

	if(*message == command_symbol)
	{	// check first char.
		char command[101];
		
		sscanf(message, "%100s", command);
		command[sizeof(command)-1] = '\0';

		size_t i;
		for (i = 0; atcommand_info[i].type < AtCommand_Unknown; ++i)
			if( 0==strcasecmp(command+1, atcommand_info[i].command+1) )
				break;

		if (atcommand_info[i].type == AtCommand_Unknown || level < atcommand_info[i].level)
		{	// doesn't return Unknown if player is normal player (display the text, not display: unknown command)
			if (level == 0)
				return AtCommand_None;
			else
				return AtCommand_Unknown;
		}
		else if( log_config.gm && (atcommand_info[i].level >= log_config.gm))
		{
			log_atcommand(sd, message);
		}
		info = atcommand_info[i];
	}
	else
	{
		return AtCommand_None;
	}
	return info.type;
}

/*==========================================
 *is_atcommand @ÉRÉ}ÉìÉhÇ…ë∂ç›Ç∑ÇÈÇ©Ç«Ç§Ç©ämîFÇ∑ÇÈ
 *------------------------------------------
 */
AtCommandType is_atcommand(const int fd, struct map_session_data &sd, const char* message, unsigned char gmlvl)
{
	const char* str = message;
	int s_flag = 0;
	AtCommandInfo info = {AtCommand_Unknown,NULL,0,NULL};
	AtCommandType type;

	if (!config.allow_atcommand_when_mute &&
		sd.sc_data[SC_NOCHAT].timer != -1)
	{
		return AtCommand_Unknown;
	}

	if (!message || !*message)
		return AtCommand_None;

	str += strlen(sd.status.name);
	while (*str && (isspace((int)((unsigned char)*str)) || (s_flag == 0 && *str == ':')))
	{
		if (*str == ':')
			s_flag = 1;
		str++;
	}
	if (!*str)
		return AtCommand_None;

	type = atcommand(info, str, sd, gmlvl > 0 ? gmlvl : sd.isGM());
	if (type != AtCommand_None)
	{
		char command[128]="";
		char output[128]="";
		const char* p = str;

		while (*p && !isspace((int)((unsigned char)*p)))
			p++;
		if( p >= str+sizeof(command)) // too long
			return AtCommand_Unknown;
		memcpy(command, str, p - str);
		command[p-str]=0;
		while (isspace((int)((unsigned char)*p)))
			p++;

		if(type == AtCommand_Unknown || info.proc == NULL)
		{
			snprintf(output, sizeof(output), msg_table[153], command); // %s is Unknown Command.
			clif_displaymessage(fd, output);
		}
		else
		{
			if( !info.proc(fd, sd, command, p) )
			{	// Command can not be executed
				snprintf(output, sizeof(output), msg_table[154], command); // %s failed.
				clif_displaymessage(fd, output);
			}
		}
		return info.type;
	}
	return AtCommand_None;
}



/*==========================================
// @ command processing functions
 *------------------------------------------
 */


///////////////////////////////////////////////////////////////////////////////
/// @send. (used for testing packet sends from the client)
/// 
/// parameters: <hex digit> <decimal digit>{20,20}
/// 
bool atcommand_send(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int type=0;
	int info[20];
	char output[128]="";
	
   	if (!message || !*message || sscanf(message, "%x %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &type, &info[1], &info[2], &info[3], &info[4], &info[5], &info[6], &info[7], &info[8], &info[9], &info[10], &info[11], &info[12], &info[13], &info[14], &info[15], &info[16], &info[17], &info[18], &info[19], &info[20]) < 1) {
		clif_displaymessage(fd, "Please enter a packet number, and - if required - up to 20 additional values.");
		return false;
	}

	if( clif_packetsend(fd, sd, type, info, sizeof(info)) )
	{
		snprintf(output, sizeof(output), msg_table[258], type, type);
		clif_displaymessage(fd, output);
	}
	else
	{
		clif_displaymessage(fd, msg_table[259]);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// @rura.
///
/// parameters: <mapname> <x> <y>
///
bool atcommand_rura(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char mapname[128]="";
	int x = 0, y = 0;
	int m;

	if (!message || !*message || sscanf(message, "%99s %d %d", mapname, &x, &y) < 1) {
		clif_displaymessage(fd, "Please, enter a map (usage: @warp/@rura/@mapmove <mapname> <x> <y>).");
		return false;
	}

	if (x <= 0)
		x = rand() % 399 + 1;
	if (y <= 0)
		y = rand() % 399 + 1;

	if (x > 0 && x < 400 && y > 0 && y < 400)
	{
		char *ip=strchr(mapname, '.');
		if(ip) *ip=0;

		m = map_mapname2mapid(mapname);
		if (m >= 0 && maps[m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM())
		{
			clif_displaymessage(fd, msg_table[247]);
			return false;
		}
		if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM())
		{
			clif_displaymessage(fd, msg_table[248]);
			return false;
		}
		if( pc_setpos(sd, mapname, x, y, 3) )
			clif_displaymessage(fd, msg_table[0]); // Warped.
		else
		{
			clif_displaymessage(fd, msg_table[1]); // Map not found.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_table[2]); // Coordinates out of range.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_where(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	char player_name[128]="";
	char output[128]="";
	
	if (!message || !*message)
		return false;
		
	if(sscanf(message, "%99[^\n]", player_name) < 1)
		return false;
	if(strncmp(sd.status.name,player_name,24)==0)
		return false;
	
	if( config.hide_GM_session && !(config.who_display_aid > 0 && sd.isGM()>=config.who_display_aid) )
	{
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) == NULL) {
		snprintf(output, sizeof(output), "%s %d %d",
			sd.mapname, sd.block_list::x, sd.block_list::y);
		clif_displaymessage(fd, output);
		return false;
	}
	snprintf(output, sizeof(output), "%s %s %d %d",
		player_name, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y);
	clif_displaymessage(fd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_jumpto(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	char player_name[128]="";
	char output[128];

	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @jumpto/@warpto/@goto <char name>).");
		return false;
	}
	if(sscanf(message, "%99[^\n]", player_name) < 1)
		return false;
	if(strncmp(sd.status.name,player_name,24)==0) //Yourself mate? Tsk tsk tsk.
		return false;

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (pl_sd->block_list::m <map_num  && maps[pl_sd->block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
			clif_displaymessage(fd, msg_table[247]);
			return false;
		}
		if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
			clif_displaymessage(fd, msg_table[248]);
			return false;
		}
		pc_setpos(sd, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y, 3);
		snprintf(output, sizeof(output), msg_table[4], player_name); // Jump to %s
		clif_displaymessage(fd, output);
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_jump(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128];
	int x = 0, y = 0;


	if(!message || !*message || sscanf(message, "%d %d", &x, &y) < 2) {
		// just nothing
	}

	if( sd.block_list::m >= map_num || 
		maps[sd.block_list::m].flag.nowarp || 
		(maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) )
	{
		clif_displaymessage(fd, msg_table[248]);
		return false;
	}
	if (x <= 0)
		x = rand() % (maps[sd.block_list::m].xs-1) + 1;
	if (y <= 0)
		y = rand() % (maps[sd.block_list::m].ys-1) + 1;
	if (x > 0 && x < maps[sd.block_list::m].xs && y > 0 && y < maps[sd.block_list::m].ys)
	{
		pc_setpos(sd, sd.mapname, x, y, 3);
		snprintf(output, sizeof(output), msg_table[5], x, y); // Jump to %d %d
		clif_displaymessage(fd, output);
	}
	else
	{
		clif_displaymessage(fd, msg_table[2]); // Coordinates out of range.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_who(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	char output[128]="";
	char match_text[104]="";
	char player_name[24]="";

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		*match_text = 0;
	basics::tolower(match_text);

	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i)
	{
		if( session[i] && (pl_sd = (struct map_session_data *)session[i]->user_session) && pl_sd->state.auth)
		{
			pl_GM_level = pl_sd->isGM();
			if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				strcpytolower(player_name,pl_sd->status.name);
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
		clif_displaymessage(fd, msg_table[28]); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[29]); // 1 player found.
	else {
		snprintf(output, sizeof(output), msg_table[30], count); // %d players found.
		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_who2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	char match_text[128]="";
	char output[128];
	char player_name[24];



	if (sscanf(message, "%99[^\n]", match_text) < 1)
		*match_text = 0;
	basics::tolower(match_text);

	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
			pl_GM_level = pl_sd->isGM();
			if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				strcpytolower(player_name,pl_sd->status.name);
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
		clif_displaymessage(fd, msg_table[28]); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[29]); // 1 player found.
	else {
		snprintf(output, sizeof(output), msg_table[30], count); // %d players found.
		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_who3(int fd, struct map_session_data &sd, const char* command, const char* message)
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



	if (sscanf(message, "%99[^\n]", match_text) < 1)
		*match_text = 0;
	basics::tolower(match_text);

	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
			pl_GM_level = pl_sd->isGM();
			if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				strcpytolower(player_name,pl_sd->status.name);
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
		clif_displaymessage(fd, msg_table[28]); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[29]); // 1 player found.
	else {
		snprintf(output, sizeof(output), msg_table[30], count); // %d players found.
		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_whomap(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	int map_id;
	char mapname[128]="", *ip;
	char output[128];


	if (!message || !*message)
		map_id = sd.block_list::m;
	else {
		sscanf(message, "%99s", mapname);
		ip=strchr(mapname, '.');
		if(ip) *ip=0;
		if ((map_id = map_mapname2mapid(mapname)) < 0)
			map_id = sd.block_list::m;
	}

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
		snprintf(output, sizeof(output), msg_table[54], maps[map_id].mapname); // No player found in map '%s'.
	else if (count == 1)
		snprintf(output, sizeof(output), msg_table[55], maps[map_id].mapname); // 1 player found in map '%s'.
	else {
		snprintf(output, sizeof(output), msg_table[56], count, maps[map_id].mapname); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_whomap2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	int map_id = 0;
	char mapname[128]="",*ip;
	char output[128];



	if (!message || !*message)
		map_id = sd.block_list::m;
	else {
		sscanf(message, "%99s", mapname);
		ip=strchr(mapname, '.');
		if(ip) *ip=0;
		if ((map_id = map_mapname2mapid(mapname)) < 0)
			map_id = sd.block_list::m;
	}

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
		snprintf(output, sizeof(output), msg_table[54], maps[map_id].mapname); // No player found in map '%s'.
	else if (count == 1)
		snprintf(output, sizeof(output), msg_table[55], maps[map_id].mapname); // 1 player found in map '%s'.
	else {
		snprintf(output, sizeof(output), msg_table[56], count, maps[map_id].mapname); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_whomap3(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char temp0[128];
	char temp1[128];
	struct map_session_data *pl_sd;
	size_t i, count;
	int pl_GM_level, GM_level;
	int map_id = 0;
	char mapname[128]="",*ip;
	char output[128];
	struct guild *g;
	struct party *p;



	if (!message || !*message)
		map_id = sd.block_list::m;
	else {
		sscanf(message, "%99s", mapname);
		ip=strchr(mapname, '.');
		if(ip) *ip=0;
		if ((map_id = map_mapname2mapid(mapname)) < 0)
			map_id = sd.block_list::m;
	}

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
		snprintf(output, sizeof(output), msg_table[54], maps[map_id].mapname); // No player found in map '%s'.
	else if (count == 1)
		snprintf(output, sizeof(output), msg_table[55], maps[map_id].mapname); // 1 player found in map '%s'.
	else {
		snprintf(output, sizeof(output), msg_table[56], count, maps[map_id].mapname); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_whogm(int fd, struct map_session_data &sd, const char* command, const char* message)
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

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		*match_text = 0;
	basics::tolower(match_text);

	count = 0;
	GM_level = sd.isGM();
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
			pl_GM_level = pl_sd->isGM();
			if (pl_GM_level > 0) {
				if (!((config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
					strcpytolower(player_name, pl_sd->status.name);
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
		clif_displaymessage(fd, msg_table[150]); // No GM found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[151]); // 1 GM found.
	else {
		snprintf(output, sizeof(output), msg_table[152], count); // %d GMs found.
		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_whozeny(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i, count,c;
	char match_text[128]="";
	char player_name[24];
	char output[128];
	CREATE_BUFFER(zeny, uint32, clif_countusers());
	CREATE_BUFFER(counted, size_t, clif_countusers());

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		*match_text = 0;
	basics::tolower(match_text);

	for(count=0, i=0; i<fd_max; ++i)
	{
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth)
		{
			strcpytolower(player_name, pl_sd->status.name);
			
			if( NULL!= strstr(player_name, match_text) )
			{	// search with no case sensitive
					zeny[count]=pl_sd->status.zeny;
					counted[i]=0;
					count++;
				}
		}
	}
	qsort(zeny, count, sizeof(uint32), hightolow_compare);

	for(c=0; c<count && c<50; ++c)
	{
		if(!zeny[c])
			continue;
		for(i=0; i<fd_max; ++i)
		{
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth && zeny[c] && counted[i]==0)
			{
				if(pl_sd->status.zeny==zeny[c])
				{
					snprintf(output, sizeof(output), "Name: %s | Zeny: %ld", pl_sd->status.name, (unsigned long)pl_sd->status.zeny);
					clif_displaymessage(fd, output);
					zeny[c]=0;
					counted[i]=1;
				}
			}
		}
	}
	if (count == 0)
		clif_displaymessage(fd, msg_table[28]); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[29]); // 1 player found.
	else
	{
		snprintf(output, sizeof(output), msg_table[30], count); // %d players found.
		clif_displaymessage(fd, output);
	}
	DELETE_BUFFER(zeny);
	DELETE_BUFFER(counted);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// cause random emote on all online players [Valaris]
///
bool atcommand_happyhappyjoyjoy(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i,e;



	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
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
bool atcommand_save(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	pc_setsavepoint(sd, sd.mapname, sd.block_list::x, sd.block_list::y);
	if (sd.status.pet_id > 0 && sd.pd)
		intif_save_petdata(sd.status.account_id, sd.pd->pet);
	pc_makesavestatus(sd);
	chrif_save(sd);
	storage_storage_save(sd);
	clif_displaymessage(fd, msg_table[6]); // Character data respawn point saved.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_load(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int m;

	m = map_mapname2mapid(sd.status.save_point.mapname);
	if (m >= 0 && maps[m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
		clif_displaymessage(fd, msg_table[249]);
		return false;
	}
	if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
		clif_displaymessage(fd, msg_table[248]);
		return false;
	}

	pc_setpos(sd, sd.status.save_point.mapname, sd.status.save_point.x, sd.status.save_point.y, 0);
	clif_displaymessage(fd, msg_table[7]); // Warping to respawn point.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_speed(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int speed;
	char output[128];


	if (!message || !*message) {
		snprintf(output, sizeof(output), "Please, enter a speed value (usage: @speed <%d-%d>).", MIN_WALK_SPEED, MAX_WALK_SPEED);
		clif_displaymessage(fd, output);
		return false;
	}

	speed = atoi(message);
	if (speed >= MIN_WALK_SPEED && speed <= MAX_WALK_SPEED) {
		sd.speed = speed;
		//Ç±ÇÃï∂Çí«â¡ by ÇÍÇ
		clif_updatestatus(sd, SP_SPEED);
		clif_displaymessage(fd, msg_table[8]); // Speed changed.
	} else {
		snprintf(output, sizeof(output), "Please, enter a valid speed value (usage: @speed <%d-%d>).", MIN_WALK_SPEED, MAX_WALK_SPEED);
		clif_displaymessage(fd, output);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_storage(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct pc_storage *stor; //changes from Freya/Yor


	if (sd.state.storage_flag == 1) {
		clif_displaymessage(fd, msg_table[250]);
		return false;
	}

	if ((stor = account2storage2(sd.status.account_id)) != NULL && stor->storage_status == 1) {
		clif_displaymessage(fd, msg_table[250]);
		return false;
	}

	storage_storageopen(sd);

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_guildstorage(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct pc_storage *stor; //changes from Freya/Yor


	if (sd.status.guild_id > 0) {
		if (sd.state.storage_flag == 1) {
			clif_displaymessage(fd, msg_table[251]);
			return false;
		}
		if ((stor = account2storage2(sd.status.account_id)) != NULL && stor->storage_status == 1) {
			clif_displaymessage(fd, msg_table[251]);
			return false;
		}
		storage_guild_storageopen(sd);
	} else {
		clif_displaymessage(fd, msg_table[252]);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_option(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int param1 = 0, param2 = 0, param3 = 0;


	if (!message || !*message || sscanf(message, "%d %d %d", &param1, &param2, &param3) < 1 || param1 < 0 || param2 < 0 || param3 < 0) {
		clif_displaymessage(fd, "Please, enter at least a option (usage: @option <param1:0+> <param2:0+> <param3:0+>).");
		return false;
	}

	sd.opt1 = param1;
	sd.opt2 = param2;
	if (!(sd.status.option & CART_MASK) && param3 & CART_MASK) {
		clif_cart_itemlist(sd);
		clif_cart_equiplist(sd);
		clif_updatestatus(sd, SP_CARTINFO);
	}
	sd.status.option = param3;
	// fix pecopeco display
	if (sd.status.class_ == 13 || sd.status.class_ == 21 || sd.status.class_ == 4014 || sd.status.class_ == 4022 || sd.status.class_ == 4030 || sd.status.class_ == 4036 || sd.status.class_ == 4037 || sd.status.class_ == 4044) {
		if (!pc_isriding(sd)) { // sd have the new value...
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
	} else {
		if (pc_isriding(sd)) { // sd have the new value...
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
	clif_displaymessage(fd, msg_table[9]); // Options changed.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_hide(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	if (sd.status.option & OPTION_HIDE) {
		sd.status.option &= ~OPTION_HIDE;
		clif_displaymessage(fd, msg_table[10]); // Invisible: Off
	} else {
		sd.status.option |= OPTION_HIDE;
		clif_displaymessage(fd, msg_table[11]); // Invisible: On
	}
	clif_changeoption(sd);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// ì]êEÇ∑ÇÈ upperÇéwíËÇ∑ÇÈÇ∆ì]ê∂Ç‚ó{éqÇ…Ç‡Ç»ÇÍÇÈ
///
bool atcommand_jobchange(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int job = 0, upper = 0;
	if (!message || !*message || sscanf(message, "%d %d", &job, &upper) < 1)
	{
		size_t i, found = 0;
		const struct { char name[16]; int id; } jobs[] = {
			{ "novice",		0 },
			{ "swordsman",	1 },
			{ "mage",		2 },
			{ "archer",		3 },
			{ "acolyte",	4 },
			{ "merchant",	5 },
			{ "thief",		6 },
			{ "knight",		7 },
			{ "priest",		8 },
			{ "priestess",	8 },
			{ "wizard",		9 },
			{ "blacksmith",	10 },
			{ "hunter",		11 },
			{ "assassin",	12 },
			{ "crusader",	14 },
			{ "monk",		15 },
			{ "sage",		16 },
			{ "rogue",		17 },
			{ "alchemist",	18 },
			{ "bard",		19 },
			{ "dancer",		20 },
			{ "super novice",	23 },
			{ "supernovice",	23 },
			{ "high novice",	4001 },
			{ "swordsman high",	4002 },
			{ "mage high",		4003 },
			{ "archer high",	4004 },
			{ "acolyte high",	4005 },
			{ "merchant high",	4006 },
			{ "thief high",		4007 },
			{ "lord knight",	4008 },
			{ "high priest",	4009 },
			{ "high priestess",	4009 },
			{ "high wizard",	4010 },
			{ "whitesmith",		4011 },
			{ "sniper",		4012 },
			{ "assassin cross",	4013 },
			{ "paladin",	4015 },
			{ "champion",	4016 },
			{ "professor",	4017 },
			{ "stalker",	4018 },
			{ "creator",	4019 },
			{ "clown",		4020 },
			{ "gypsy",		4021 },
			{ "baby novice",	4023 },
			{ "baby swordsman",	4024 },
			{ "baby mage",		4025 },
			{ "baby archer",	4026 },
			{ "baby acolyte",	4027 },
			{ "baby merchant",	4028 },
			{ "baby thief",		4029 },
			{ "baby knight",	4030 },
			{ "baby priest",	4031 },
			{ "baby priestess",	4031 },
			{ "baby wizard",	4032 },
			{ "baby blacksmith",4033 },
			{ "baby hunter",	4034 },
			{ "baby assassin",	4035 },
			{ "baby crusader",	4037 },
			{ "baby monk",		4038 },
			{ "baby sage",		4039 },
			{ "baby rogue",		4040 },
			{ "baby alchemist",	4041 },
			{ "baby bard",		4042 },
			{ "baby dancer",	4043 },
			{ "super baby",		4045 },
			{ "taekwon",		4046 },
			{ "taekwon boy",	4046 },
			{ "taekwon girl",	4046 },
			{ "star gladiator",	4047 },
			{ "soul linker",	4049 },
		};

		for (i=0; i < (sizeof(jobs) / sizeof(jobs[0])); ++i)
		{
			if( 0==strncasecmp(message, jobs[i].name, 16) )
			{
				job = jobs[i].id;
				upper = 0;
				found = 1;
				break;
			}
		}
		if (!found)
		{
			clif_displaymessage(fd, "Please, enter job ID (usage: @job/@jobchange <job ID>).");
			return false;
		}
	}

	if (job == 37 ||job == 45)
		return true;

	if ((job >= 0 && job < MAX_PC_CLASS))
	{
		int j;
		// fix pecopeco display
		if ((job != 13 && job != 21 && job != 4014 && job != 4022 && job != 4030 && job != 4036 && job != 4037 && job != 4044 )) {
			if (pc_isriding(sd)) {
				if (sd.status.class_ == 13)
					sd.status.class_ = sd.view_class = 7;
				if (sd.status.class_ == 21)
					sd.status.class_ = sd.view_class = 14;
				if (sd.status.class_ == 4014)
					sd.status.class_ = sd.view_class = 4008;
				if (sd.status.class_ == 4022)
					sd.status.class_ = sd.view_class = 4015;
				if (sd.status.class_ == 4036)
					sd.status.class_ = sd.view_class = 4030;
				if (sd.status.class_ == 4044)
					sd.status.class_ = sd.view_class = 4037;
				sd.status.option &= ~0x0020;
				clif_changeoption(sd);
				status_calc_pc(sd, 0);
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
			if(sd.status.inventory[j].nameid>0 && sd.status.inventory[j].equip!=0)
				pc_unequipitem(sd, j, 3);
		}
		if (pc_jobchange(sd, job, upper) == 0)
			clif_displaymessage(fd, msg_table[12]); // Your job has been changed.
		else {
			clif_displaymessage(fd, msg_table[155]); // Impossible to change your job.
			return false;
		}
	} else {
		clif_displaymessage(fd, "Please, enter a valid job ID (usage: @job/@jobchange <job ID>).");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_die(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	clif_specialeffect(sd,450,1);
	pc_damage(sd, sd.status.hp + 1,NULL);
	clif_displaymessage(fd, msg_table[13]); // A pity! You've died.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_kill(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;



	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @kill <char name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can kill only lower or same level
			pc_damage(*pl_sd, pl_sd->status.hp + 1,NULL);
			clif_displaymessage(fd, msg_table[14]); // Character killed.
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_alive(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	if( pc_isdead(sd) )
	{
		sd.status.hp = sd.status.max_hp;
		sd.status.sp = sd.status.max_sp;
		clif_skill_nodamage(sd,sd,ALL_RESURRECTION,4,1);
	pc_setstand(sd);
	if (config.pc_invincible_time > 0)
		pc_setinvincibletimer(sd, config.pc_invincible_time);
	clif_updatestatus(sd, SP_HP);
	clif_updatestatus(sd, SP_SP);
		clif_resurrection(sd, 1);
	clif_displaymessage(fd, msg_table[16]); // You've been revived! It's a miracle!
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_kami(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128]="";

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a message (usage: @kami <message>).");
		return false;
	}
	sscanf(message, "%199[^\n]", output);
	intif_GMmessage(output, 1+strlen(output), (command[5]=='b') ? 0x10 : 0);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_heal(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	long hp = 0, sp = 0; // [Valaris] thanks to fov
	sscanf(message, "%ld %ld", &hp, &sp);
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
		pc_heal(sd, hp, sp);
		if (hp >= 0 && sp >= 0)
			clif_displaymessage(fd, msg_table[17]); // HP, SP recovered.
		else
			clif_displaymessage(fd, msg_table[156]); // HP or/and SP modified.
	}
	else
	{
		clif_displaymessage(fd, msg_table[157]); // HP and SP are already with the good value.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @item command (usage: @item <name/id_of_item> <quantity>) (modified by [Yor] for pet_egg)
///
bool atcommand_item(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char item_name[128]="";
	int number = 0, item_id=0, flag;
	struct item item_tmp;
	struct item_data *item_data;
	int get_count, i, pet_id;

	if( !message || !*message || sscanf(message, "%99s %d", item_name, &number) < 1)
	{
		clif_displaymessage(fd, "Please, enter an item name/id (usage: @item <item name or ID> [quantity]).");
		return false;
	}
	
	if (number <= 0)
		number = 1;
	
	if( (item_data = itemdb_searchname(item_name)) != NULL ||
		(item_data = itemdb_exists(atoi(item_name))) != NULL )
		item_id = item_data->nameid;
	
	if (item_id >= 500)
	{
		get_count = number;
		// check pet egg
		pet_id = search_petDB_index(item_id, PET_EGG);
		if( item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8)
		{
			get_count = 1;
		}
		for(i=0; i<number; i+=get_count)
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
				item_tmp = item(item_id);
				if( (flag = pc_additem(sd, item_tmp, get_count)) )
					clif_additem(sd, 0, 0, flag);
			}
		}
		clif_displaymessage(fd, msg_table[18]); // Item created.
	} else {
		clif_displaymessage(fd, msg_table[19]); // Invalid item ID or name.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_item2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct item item_tmp;
	struct item_data *item_data;
	char item_name[128]="";
	int item_id, number = 0;
	int identify = 0, refine = 0, attr = 0;
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
	int flag;
	int loop, get_count, i;

	if (!message || !*message || sscanf(message, "%99s %d %d %d %d %d %d %d %d", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4) < 1)
	{
		clif_displaymessage(fd, "Please, enter all informations (usage: @item2 <item name or ID> <quantity>");
		clif_displaymessage(fd, "  <Identify_flag> <refine> <attribut> <Card1> <Card2> <Card3> <Card4>).");
		return false;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id > 500) {
		loop = 1;
		get_count = number;
		if (item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8) {
			loop = number;
			get_count = 1;
			if (item_data->type == 7) {
				identify = 1;
				refine = 0;
			}
			if (item_data->type == 8)
				refine = 0;
			if (refine > 10)
				refine = 10;
		} else {
			identify = 1;
			refine = attr = 0;
		}
		for (i = 0; i < loop; ++i)
		{
			item_tmp = item(item_id);
			item_tmp.identify = identify;
			item_tmp.refine = refine;
			item_tmp.attribute = attr;
			item_tmp.card[0] = c1;
			item_tmp.card[1] = c2;
			item_tmp.card[2] = c3;
			item_tmp.card[3] = c4;
			if ((flag = pc_additem(sd, item_tmp, get_count)))
				clif_additem(sd, 0, 0, flag);
		}
		clif_displaymessage(fd, msg_table[18]); // Item created.
	} else {
		clif_displaymessage(fd, msg_table[19]); // Invalid item ID or name.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_itemreset(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int i;


	for (i = 0; i < MAX_INVENTORY; ++i) {
		if (sd.status.inventory[i].amount && sd.status.inventory[i].equip == 0)
			pc_delitem(sd, i, sd.status.inventory[i].amount, 0);
	}
	clif_displaymessage(fd, msg_table[20]); // All of your items have been removed.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_itemcheck(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	pc_checkitem(sd);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_baselevelup(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int level, i;


	if (!message || !*message || (level = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement (usage: @lvup/@blevel/@baselvlup <number of levels>).");
		return false;
	}


	if (level > 0) {
		if (sd.status.base_level == config.maximum_level) {	// check for max level by Valaris
			clif_displaymessage(fd, msg_table[47]); // Base level can't go any higher.
			return false;
		}	// End Addition
		if((size_t)level > config.maximum_level || (size_t)level > (config.maximum_level - sd.status.base_level)) // fix positiv overflow
			level = config.maximum_level - sd.status.base_level;
		for (i = 1; i <= level; ++i)
			sd.status.status_point += (sd.status.base_level + i + 14) / 5;
		sd.status.base_level += level;
		clif_updatestatus(sd, SP_BASELEVEL);
		clif_updatestatus(sd, SP_NEXTBASEEXP);
		clif_updatestatus(sd, SP_STATUSPOINT);
		status_calc_pc(sd, 0);
		pc_heal(sd, sd.status.max_hp, sd.status.max_sp);
		clif_misceffect(sd, 0);
		clif_displaymessage(fd, msg_table[21]); // Base level raised.
	} else {
		if (sd.status.base_level == 1) {
			clif_displaymessage(fd, msg_table[158]); // Base level can't go any lower.
			return false;
		}
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
		clif_displaymessage(fd, msg_table[22]); // Base level lowered.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_joblevelup(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int up_level = 50;
	int level;
	struct pc_base_job s_class;

	s_class = pc_calc_base_job(sd.status.class_);

	if (!message || !*message || (level = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement (usage: @joblvup/@jlevel/@joblvlup <number of levels>).");
		return false;
	}

	if (s_class.job == 0)
		up_level -= 40;
	// super novices can go up to 99 [celest]
	else if (s_class.job == 23)
		up_level += 49;
	else if (sd.status.class_ > 4007 && sd.status.class_ < 4023)
		up_level += 20;

	if (level > 0) {
		if (sd.status.job_level == up_level) {
			clif_displaymessage(fd, msg_table[23]); // Job level can't go any higher.
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
		clif_displaymessage(fd, msg_table[24]); // Job level raised.
	} else {
		if (sd.status.job_level == 1) {
			clif_displaymessage(fd, msg_table[159]); // Job level can't go any lower.
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
		clif_displaymessage(fd, msg_table[25]); // Job level lowered.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_help(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char buf[2048], w1[2048], w2[2048];
	int i, gm_level;
	FILE* fp;


	memset(buf, '\0', sizeof(buf));

	if((fp = basics::safefopen(help_txt, "r")) != NULL) {
		clif_displaymessage(fd, msg_table[26]); // Help commands:
		gm_level = sd.isGM();
		while(fgets(buf, sizeof(buf), fp) != NULL) {
			if (buf[0] == '/' && buf[1] == '/')
				continue;
			for (i = 0; buf[i] != '\0'; ++i) {
				if (buf[i] == '\r' || buf[i] == '\n') {
					buf[i] = '\0';
					break;
				}
			}
			if (sscanf(buf, "%2047[^:]:%2047[^\n]", w1, w2) < 2)
				clif_displaymessage(fd, buf);
			else if (gm_level >= atoi(w1))
				clif_displaymessage(fd, w2);
		}
		fclose(fp);
	} else {
		clif_displaymessage(fd, msg_table[27]); // File help.txt not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_gm(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char password[128];


	memset(password, '\0', sizeof(password));

	if (!message || !*message || sscanf(message, "%99[^\n]", password) < 1) {
		clif_displaymessage(fd, "Please, enter a password (usage: @gm <password>).");
		return false;
	}

	if (sd.isGM()) { // a GM can not use this function. only a normal player (become gm is not for gm!)
		clif_displaymessage(fd, msg_table[50]); // You already have some GM powers.
		return false;
	} else
		chrif_changegm(sd.status.account_id, password, strlen(password) + 1);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_pvpoff(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i;


	if (config.pk_mode) { //disable command if server is in PK mode [Valaris]
		clif_displaymessage(fd, msg_table[52]); // This option cannot be used in PK Mode.
		return false;
	}

	if (maps[sd.block_list::m].flag.pvp) {
		maps[sd.block_list::m].flag.pvp = 0;
		clif_send0199(sd.block_list::m, 0);
		for (i = 0; i < fd_max; ++i) {	//êlêîï™ÉãÅ[Év
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
		clif_displaymessage(fd, msg_table[31]); // PvP: Off.
	} else {
		clif_displaymessage(fd, msg_table[160]); // PvP is already Off.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_pvpon(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i;


	if (config.pk_mode) { //disable command if server is in PK mode [Valaris]
		clif_displaymessage(fd, msg_table[52]); // This option cannot be used in PK Mode.
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
		clif_displaymessage(fd, msg_table[32]); // PvP: On.
	} else {
		clif_displaymessage(fd, msg_table[161]); // PvP is already On.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_gvgoff(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	if (maps[sd.block_list::m].flag.gvg) {
		maps[sd.block_list::m].flag.gvg = 0;
		clif_send0199(sd.block_list::m, 0);
		clif_displaymessage(fd, msg_table[33]); // GvG: Off.
	} else {
		clif_displaymessage(fd, msg_table[162]); // GvG is already Off.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_gvgon(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	if (!maps[sd.block_list::m].flag.gvg) {
		maps[sd.block_list::m].flag.gvg = 1;
		clif_send0199(sd.block_list::m, 3);
		clif_displaymessage(fd, msg_table[34]); // GvG: On.
	} else {
		clif_displaymessage(fd, msg_table[163]); // GvG is already On.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///
bool atcommand_model(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	unsigned int hair_style = 0, hair_color = 0, cloth_color = 0;
	char output[128];



	if (!message || !*message || sscanf(message, "%d %d %d", &hair_style, &hair_color, &cloth_color) < 1) {
		snprintf(output, sizeof(output), "Please, enter at least a value (usage: @model <hair ID: %ld-%ld> <hair color: %ld-%ld> <clothes color: %ld-%ld>).",
		        (unsigned long)MIN_HAIR_STYLE, (unsigned long)MAX_HAIR_STYLE, (unsigned long)MIN_HAIR_COLOR, (unsigned long)MAX_HAIR_COLOR, (unsigned long)MIN_CLOTH_COLOR, (unsigned long)MAX_CLOTH_COLOR);
		clif_displaymessage(fd, output);
		return false;
	}

	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE &&
		hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR &&
		cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
		//ïbÃêFïœçX
		if (cloth_color != 0 && sd.status.sex == 1 && (sd.status.class_ == 12 ||  sd.status.class_ == 17)) {
			//ïbÃêFñ¢é¿ëïêEÇÃîªíË
			clif_displaymessage(fd, msg_table[35]); // You can't use this command with this class_.
			return false;
		} else {
			pc_changelook(sd, LOOK_HAIR, hair_style);
			pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
			pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
			clif_displaymessage(fd, msg_table[36]); // Appearence changed.
		}
	} else {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @dye && @ccolor
///
bool atcommand_dye(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	unsigned int cloth_color = 0;
	char output[128];


	if (!message || !*message || sscanf(message, "%d", &cloth_color) < 1) {
		snprintf(output, sizeof(output), "Please, enter a clothes color (usage: @dye/@ccolor <clothes color: %ld-%ld>).", (unsigned long)MIN_CLOTH_COLOR, (unsigned long)MAX_CLOTH_COLOR);
		clif_displaymessage(fd, output);
		return false;
	}

	if (cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
		pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
		clif_displaymessage(fd, msg_table[36]); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @hairstyle && @hstyle
///
bool atcommand_hair_style(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	unsigned int hair_style = 0;
	char output[128];


	if (!message || !*message || sscanf(message, "%d", &hair_style) < 1) {
		snprintf(output, sizeof(output), "Please, enter a hair style (usage: @hairstyle/@hstyle <hair ID: %ld-%ld>).", (unsigned long)MIN_HAIR_STYLE, (unsigned long)MAX_HAIR_STYLE);
		clif_displaymessage(fd, output);
		return false;
	}

	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE) {
		if (hair_style != 0 && sd.status.sex == 1 && (sd.status.class_ == 12 || sd.status.class_ == 17)) {
			clif_displaymessage(fd, msg_table[35]); // You can't use this command with this class_.
			return false;
		} else {
			pc_changelook(sd, LOOK_HAIR, hair_style);
			clif_displaymessage(fd, msg_table[36]); // Appearence changed.
		}
	} else {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @charhairstyle by [MouseJstr]
///
bool atcommand_charhairstyle(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @haircolor && @hcolor
bool atcommand_hair_color(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	unsigned int hair_color = 0;
	char output[128];


	if (!message || !*message || sscanf(message, "%d", &hair_color) < 1) {
		snprintf(output, sizeof(output), "Please, enter a hair color (usage: @haircolor/@hcolor <hair color: %ld-%ld>).", (unsigned long)MIN_HAIR_COLOR, (unsigned long)MAX_HAIR_COLOR);
		clif_displaymessage(fd, output);
		return false;
	}

	if (hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR) {
		if (hair_color != 0 && sd.status.sex == 1 && (sd.status.class_ == 12 || sd.status.class_ == 17)) {
			clif_displaymessage(fd, msg_table[35]); // You can't use this command with this class_.
			return false;
		} else {
			pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
			clif_displaymessage(fd, msg_table[36]); // Appearence changed.
		}
	} else {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @go [city_number/city_name]: improved by [yor] to add city names and help
bool atcommand_go(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int town;
	char mapname[128]="", *ip;
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



	if(maps[sd.block_list::m].flag.nogo) {
		clif_displaymessage(sd.fd,"You can not use @go on this map.");
		return true;
	}
	// get the number
	town = atoi(message);

	// if no value, display all value
	if (!message || !*message || sscanf(message, "%99s", mapname) < 1 || town < -3 || town >= (int)(sizeof(data) / sizeof(data[0]))) {
		clif_displaymessage(fd, msg_table[38]); // Invalid location number or name.
		clif_displaymessage(fd, msg_table[82]); // Please, use one of this number/name:
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

		mapname[sizeof(mapname)-1] = '\0';
		basics::tolower(mapname);
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
					clif_displaymessage(fd, msg_table[247]);
					return false;
				}
				else if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM())
				{
					clif_displaymessage(fd, msg_table[248]);
					return false;
				}
				else if( pc_setpos(sd, sd.status.memo_point[-town-1].mapname, sd.status.memo_point[-town-1].x, sd.status.memo_point[-town-1].y, 3) )
				{
					clif_displaymessage(fd, msg_table[0]); // Warped.
				}
				else
				{
					clif_displaymessage(fd, msg_table[1]); // Map not found.
					return false;
				}
			}
			else
			{
				snprintf(output, sizeof(output), msg_table[164], -town-1); // Your memo point #%d doesn't exist.
				clif_displaymessage(fd, output);
				return false;
			}
		}
		else if (town >= 0 && town < (int)(sizeof(data) / sizeof(data[0])))
		{
			m = map_mapname2mapid(data[town].map);
			if (m >= 0 && maps[m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
				clif_displaymessage(fd, msg_table[247]);
				return false;
			}
			if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
				clif_displaymessage(fd, msg_table[248]);
				return false;
			}
			if( pc_setpos(sd, (char *)data[town].map, data[town].x, data[town].y, 3) ) {
				clif_displaymessage(fd, msg_table[0]); // Warped.
			} else {
				clif_displaymessage(fd, msg_table[1]); // Map not found.
				return false;
			}
		}
		else
		{	// if you arrive here, you have an error in town variable when reading of names
			clif_displaymessage(fd, msg_table[38]); // Invalid location number or name.
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_monster(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char name[128]="";
	char monster[128]="";
	char output[128];
	uint32 mob_id;
	unsigned int number = 0;
	unsigned int x = 0, y = 0;
	unsigned int count;
	unsigned int i, j, k;
	unsigned int mx, my, range;



	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\" %99s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	     sscanf(message, "%99s \"%[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	     sscanf(message, "%99s %99s %d %d %d", name, monster, &number, &x, &y) < 2)) {
		clif_displaymessage(fd, msg_table[80]); // Give a display name and monster name/id please.
		return false;
	}

	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_table[40]); // Invalid monster ID or name.
		return false;
	}
	if (mob_id == MOBID_EMPERIUM) {
		clif_displaymessage(fd, msg_table[83]); // Cannot spawn emperium.
		return false;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
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
			clif_displaymessage(fd, msg_table[39]); // All monster summoned!
		else {
			snprintf(output, sizeof(output), msg_table[240], count); // %d monster(s) summoned!
			clif_displaymessage(fd, output);
		}
	else {
		clif_displaymessage(fd, msg_table[40]); // Invalid monster ID or name.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_spawn(int fd, struct map_session_data &sd, const char* command, const char* message) {
	char name[128]="";
	char monster[128]="";
	char output[128];
	uint32 mob_id;
	unsigned int number = 0;
	unsigned int x = 0, y = 0;
	unsigned int count;
	unsigned int i, j, k;
	unsigned int mx, my, range;



	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\" %99s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	     sscanf(message, "%99s \"%[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	     sscanf(message, "%99s %d %99s %d %d", monster, &number, name, &x, &y) < 1)) {
		clif_displaymessage(fd, msg_table[143]); // Give a monster name/id please.
		return false;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_table[40]); // Invalid monster ID or name.
		return false;
	}

	if (mob_id == MOBID_EMPERIUM) {
		clif_displaymessage(fd, msg_table[83]); // Cannot spawn emperium.
		return false;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
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
			clif_displaymessage(fd, msg_table[39]); // All monster summoned!
		else
		{
			snprintf(output, sizeof(output), msg_table[240], count); // %d monster(s) summoned!
			clif_displaymessage(fd, output);
		}
	}
	else
	{
		clif_displaymessage(fd, msg_table[40]); // Invalid monster ID or name.
		return false;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////
///
///  small monster spawning [Valaris]
bool atcommand_monstersmall(int fd, struct map_session_data &sd, const char* command, const char* message) {
	char name[128] = "";
	char monster[128] = "";
	uint32 mob_id = 0;
	unsigned int number = 0;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int count;
	unsigned int i;



	if (!message || !*message) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return true;
	}

	if (sscanf(message, "\"%[^\"]\" %99s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	    sscanf(message, "%99s \"%[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	    sscanf(message, "%99s %d %99s %d %d", monster, &number, name, &x, &y) < 1) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return true;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(monster);

	if (mob_id == 0) {
		clif_displaymessage(fd, "Monster name hasn't been found.");
		return true;
	}

	if (mob_id == MOBID_EMPERIUM) {
		clif_displaymessage(fd, "Cannot spawn emperium.");
		return true;
	}

	if (mob_id > 2000) {
		clif_displaymessage(fd, "Invalid monster ID"); // Invalid Monster ID.
		return true;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
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
		clif_displaymessage(fd, msg_table[39]); // Monster Summoned!!
	else
		clif_displaymessage(fd, msg_table[40]); // Invalid Monster ID.

	return true;
}
///////////////////////////////////////////////////////////////////////////////
///
///  big monster spawning [Valaris]
bool atcommand_monsterbig(int fd, struct map_session_data &sd, const char* command, const char* message) {
	char name[128] = "";
	char monster[128] = "";
	uint32 mob_id = 0;
	unsigned int number = 0;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int count;
	unsigned int i;



	if (!message || !*message) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return true;
	}

	if (sscanf(message, "\"%[^\"]\" %99s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	    sscanf(message, "%99s \"%[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	    sscanf(message, "%99s %d %99s %d %d", monster, &number, name, &x, &y) < 1) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return true;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(monster);

	if (mob_id == 0) {
		clif_displaymessage(fd, "Monster name hasn't been found.");
		return true;
	}

	if (mob_id == MOBID_EMPERIUM) {
		clif_displaymessage(fd, "Cannot spawn emperium.");
		return true;
	}

	if (mob_id > 2000) {
		clif_displaymessage(fd, "Invalid monster ID"); // Invalid Monster ID.
		return true;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
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
		clif_displaymessage(fd, msg_table[39]); // Monster Summoned!!
	else
		clif_displaymessage(fd, msg_table[40]); // Invalid Monster ID.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
class CAtKillMonster : public CMapProcessor
{
	int flag;
public:
	CAtKillMonster(int f) : flag(f)	{}
		~CAtKillMonster()			{}
	virtual int process(struct block_list& bl) const
	{
		struct mob_data &md = (struct mob_data &)bl;
		if(flag)
			mob_damage(md, md.hp, 2, NULL);
		else
			mob_remove_map(md,1);
		return 0;
	}
};

bool atcommand_killmonster_sub(int fd, struct map_session_data &sd, const char* message, const int drop)
{
	int map_id;
	char mapname[128], *ip;

	memset(mapname, '\0', sizeof(mapname));

	if (!message || !*message || sscanf(message, "%99s", mapname) < 1)
		map_id = sd.block_list::m;
	else
	{
		ip = strchr(mapname, '.');
		if(ip) *ip=0;
		if ((map_id = map_mapname2mapid(mapname)) < 0)
			map_id = sd.block_list::m;
	}

	if(map_id>0 && map_id<(int)map_num)
	{
		CMap::foreachinarea( CAtKillMonster(drop),
			map_id, 0, 0, maps[map_id].xs-1, maps[map_id].ys-1, BL_MOB);
//		map_foreachinarea(atkillmonster_sub, 
//			map_id, 0, 0, maps[map_id].xs-1, maps[map_id].ys-1, BL_MOB,
//			drop);
		clif_displaymessage(fd, msg_table[165]); // All monsters killed!
		return true;
}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_killmonster(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	return atcommand_killmonster_sub(fd, sd, message, 1);
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_killmonster2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	return atcommand_killmonster_sub(fd, sd, message, 0);
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_refine(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int i, position = 0, refine = 0, current_position, final_refine;
	int count;
	char output[128];


	if (!message || !*message || sscanf(message, "%d %d", &position, &refine) < 2) {
		clif_displaymessage(fd, "Please, enter a position and a amount (usage: @refine <equip position> <+/- amount>).");
		return false;
	}

	if (refine < -10)
		refine = -10;
	else if (refine > 10)
		refine = 10;
	else if (refine == 0)
		refine = 1;

	count = 0;
	for (i = 0; i < MAX_INVENTORY; ++i) {
		if (sd.status.inventory[i].nameid &&	// äYìñå¬èäÇÃëïîıÇê∏òBÇ∑ÇÈ
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
		clif_displaymessage(fd, msg_table[166]); // No item has been refined!
	else if (count == 1)
		clif_displaymessage(fd, msg_table[167]); // 1 item has been refined!
	else {
		snprintf(output, sizeof(output), msg_table[168], count); // %d items have been refined!
		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_produce(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char item_name[128]="";
	char output[128];
	int item_id, attribute = 0, star = 0;
	int flag = 0;
	struct item_data *item_data;
	struct item tmp_item;


	if (!message || !*message || sscanf(message, "%99s %d %d", item_name, &attribute, &star) < 1) {
		clif_displaymessage(fd, "Please, enter at least an item name/id (usage: @produce <equip name or equip ID> <element> <# of very's>).");
		return false;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
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
		clif_produceeffect(sd, item_id, 0); // êªë¢ÉGÉtÉFÉNÉgÉpÉPÉbÉg
		clif_misceffect(sd, 3); // ëºêlÇ…Ç‡ê¨å˜Çí ím
		if ((flag = pc_additem(sd, tmp_item, 1)))
			clif_additem(sd, 0, 0, flag);
	} else {
		if (config.error_log)
			ShowMessage("@produce NOT WEAPON [%d]\n", item_id);
		if (item_id != 0 && itemdb_exists(item_id))
			snprintf(output, sizeof(output), msg_table[169], item_id, item_data->name); // This item (%d: '%s') is not an equipment.
		else
			snprintf(output, sizeof(output), msg_table[170]); // This item is not an equipment.
		clif_displaymessage(fd, output);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Sub-function to display actual memo points
void atcommand_memo_sub(struct map_session_data& sd)
{
	int i;
	char output[128];

	clif_displaymessage(sd.fd,  "Your actual memo positions are (except respawn point):");
	for (i = MIN_PORTAL_MEMO; i <= MAX_PORTAL_MEMO; ++i) {
		if (sd.status.memo_point[i].mapname[0])
			snprintf(output, sizeof(output), "%d - %s (%d,%d)", i, sd.status.memo_point[i].mapname, sd.status.memo_point[i].x, sd.status.memo_point[i].y);
		else
			snprintf(output, sizeof(output), msg_table[171], i); // %d - void
		clif_displaymessage(sd.fd, output);
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_memo(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128];
	int position = 0;


	if (!message || !*message || sscanf(message, "%d", &position) < 1)
		atcommand_memo_sub(sd);
	else {
		if (position >= MIN_PORTAL_MEMO && position <= MAX_PORTAL_MEMO) {
			if( sd.block_list::m < map_num && 
				(maps[sd.block_list::m].flag.nowarpto || maps[sd.block_list::m].flag.nomemo) && 
				config.any_warp_GM_min_level > sd.isGM() )
			{
				clif_displaymessage(fd, msg_table[253]);
				return false;
			}
			if (sd.status.memo_point[position].mapname[0]) {
				snprintf(output, sizeof(output), msg_table[172], position, sd.status.memo_point[position].mapname, sd.status.memo_point[position].x, sd.status.memo_point[position].y); // You replace previous memo position %d - %s (%d,%d).
				clif_displaymessage(fd, output);
			}
			memcpy(sd.status.memo_point[position].mapname, maps[sd.block_list::m].mapname, 24);
			sd.status.memo_point[position].x = sd.block_list::x;
			sd.status.memo_point[position].y = sd.block_list::y;
			clif_skill_memo(sd, 0);
			if (pc_checkskill(sd, AL_WARP) <= (position + 1))
				clif_displaymessage(fd, msg_table[173]); // Note: you don't have the 'Warp' skill level to use it.
			atcommand_memo_sub(sd);
		} else {
			snprintf(output, sizeof(output), "Please, enter a valid position (usage: @memo <memo_position:%d-%d>).", MIN_PORTAL_MEMO, MAX_PORTAL_MEMO);
			clif_displaymessage(fd, output);
			atcommand_memo_sub(sd);
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_gat(int fd, struct map_session_data &sd, const char* command, const char* message)
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
bool atcommand_packet(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	static int packet_mode = 0;
	int i, x = 0, y = 0;


	if (strstr(command, "packetmode")) {
		packet_mode = atoi(message);
		clif_displaymessage(fd, "Packet mode changed.");
		return true;
	}
	
	if (!message || !*message || (i = sscanf(message, "%d %d", &x, &y)) < 1) {
		clif_displaymessage(fd, "Please, enter a status type/flag (usage: @packet <status type> <flag>).");
		return false;
	}
	if (i == 1) y = 1;

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

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @stpoint (Rewritten by [Yor])

bool atcommand_statuspoint(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int point, new_status_point;

	if (!message || !*message || (point = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter a number (usage: @stpoint <number of points>).");
		return false;
	}

	new_status_point = (int)sd.status.status_point + point;
	if (point > 0 && (point > 0x7FFF || new_status_point > 0x7FFF)) // fix positiv overflow
		new_status_point = 0x7FFF;
	else if (point < 0 && (point < -0x7FFF || new_status_point < 0)) // fix negativ overflow
		new_status_point = 0;

	if (new_status_point != (int)sd.status.status_point) {
		sd.status.status_point = (short)new_status_point;
		clif_updatestatus(sd, SP_STATUSPOINT);
		clif_displaymessage(fd, msg_table[174]); // Number of status points changed!
	} else {
		if (point < 0)
			clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @skpoint (Rewritten by [Yor])

bool atcommand_skillpoint(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int point, new_skill_point;


	if (!message || !*message || (point = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter a number (usage: @skpoint <number of points>).");
		return false;
	}

	new_skill_point = (int)sd.status.skill_point + point;
	if (point > 0 && (point > 0x7FFF || new_skill_point > 0x7FFF)) // fix positiv overflow
		new_skill_point = 0x7FFF;
	else if (point < 0 && (point < -0x7FFF || new_skill_point < 0)) // fix negativ overflow
		new_skill_point = 0;

	if (new_skill_point != (int)sd.status.skill_point) {
		sd.status.skill_point = (short)new_skill_point;
		clif_updatestatus(sd, SP_SKILLPOINT);
		clif_displaymessage(fd, msg_table[175]); // Number of skill points changed!
	} else {
		if (point < 0)
			clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @zeny (Rewritten by [Yor])

bool atcommand_zeny(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	long zeny;
	uint32 new_zeny;

	if (!message || !*message || (zeny = atoi(message)) == 0)
	{
		clif_displaymessage(fd, "Please, enter an amount (usage: @zeny <amount>).");
		return false;
	}

	new_zeny = sd.status.zeny + zeny;
	if( zeny>0 && (new_zeny<sd.status.zeny || new_zeny>MAX_ZENY) ) // pos overflow & max
		new_zeny = MAX_ZENY;
	else if( zeny<0 && new_zeny>sd.status.zeny ) // neg overflow
		new_zeny = 0;

	if (new_zeny != sd.status.zeny)
	{
		sd.status.zeny = new_zeny;
		clif_updatestatus(sd, SP_ZENY);
		clif_displaymessage(fd, msg_table[176]); // Number of zenys changed!
		return true;
	}
	else
	{
		if (zeny < 0)
			clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_param(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int i, index, value = 0, new_value;
	char output[128];
	const char* param[] = { "@str", "@agi", "@vit", "@int", "@dex", "@luk", NULL };
	unsigned short* status[] = {
		&sd.status.str,  &sd.status.agi, &sd.status.vit,
		&sd.status.int_, &sd.status.dex, &sd.status.luk
	};


	if (!message || !*message || sscanf(message, "%d", &value) < 1 || value == 0) {
		snprintf(output, sizeof(output), "Please, enter a valid value (usage: @str,@agi,@vit,@int,@dex,@luk <+/-adjustement>).");
		clif_displaymessage(fd, output);
		return false;
	}

	index = -1;
	for (i = 0; param[i] != NULL; ++i) {
		if(strcasecmp(command, param[i]) == 0) {
			index = i;
			break;
		}
	}
	if (index < 0 || index > MAX_STATUS_TYPE) { // normaly impossible...
		snprintf(output, sizeof(output), "Please, enter a valid value (usage: @str,@agi,@vit,@int,@dex,@luk <+/-adjustement>).");
		clif_displaymessage(fd, output);
		return false;
	}

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
		clif_displaymessage(fd, msg_table[42]); // Stat changed.
	} else {
		if (value < 0)
			clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Stat all by fritz (rewritten by [Yor])
bool atcommand_stat_all(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int index, count, value = 0, new_value;
	unsigned short* status[] = {
		&sd.status.str,  &sd.status.agi, &sd.status.vit,
		&sd.status.int_, &sd.status.dex, &sd.status.luk
	};


	if (!message || !*message || sscanf(message, "%d", &value) < 1 || value == 0)
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
		clif_displaymessage(fd, msg_table[84]); // All stats changed!
	else {
		if (value < 0)
			clif_displaymessage(fd, msg_table[177]); // Impossible to decrease a stat.
		else
			clif_displaymessage(fd, msg_table[178]); // Impossible to increase a stat.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_guildlevelup(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int level = 0;
	short added_level;
	struct guild *guild_info;


	if (!message || !*message || sscanf(message, "%d", &level) < 1 || level == 0) {
		clif_displaymessage(fd, "Please, enter a valid level (usage: @guildlvup/@guildlvlup <# of levels>).");
		return false;
	}

	if (sd.status.guild_id <= 0 || (guild_info = guild_search(sd.status.guild_id)) == NULL) {
		clif_displaymessage(fd, msg_table[43]); // You're not in a guild.
		return false;
	}
	if (strcmp(sd.status.name, guild_info->master) != 0) {
		clif_displaymessage(fd, msg_table[44]); // You're not the master of your guild.
		return false;
	}

	added_level = (short)level;
	if (level > 0 && (level > MAX_GUILDLEVEL || added_level > ((short)MAX_GUILDLEVEL - guild_info->guild_lv))) // fix positiv overflow
		added_level = (short)MAX_GUILDLEVEL - guild_info->guild_lv;
	else if (level < 0 && (level < -MAX_GUILDLEVEL || added_level < (1 - guild_info->guild_lv))) // fix negativ overflow
		added_level = 1 - guild_info->guild_lv;

	if (added_level != 0) {
		intif_guild_change_basicinfo(guild_info->guild_id, GBI_GUILDLV, added_level);
		clif_displaymessage(fd, msg_table[179]); // Guild level changed.
	} else {
		clif_displaymessage(fd, msg_table[45]); // Guild level change failed.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_makeegg(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct item_data *item_data;
	int id, pet_id;


	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a monter/egg name/id (usage: @makeegg <pet_id>).");
		return false;
	}

	if ((item_data = itemdb_searchname(message)) != NULL) // for egg name
		id = item_data->nameid;
	else if ((id = mobdb_searchname(message)) == 0) // for monster name
		id = atoi(message);

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
		clif_displaymessage(fd, msg_table[180]); // The monter/egg name/id doesn't exist.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_hatch(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	if (sd.status.pet_id <= 0)
		clif_sendegg(sd);
	else {
		clif_displaymessage(fd, msg_table[181]); // You already have a pet.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_petfriendly(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int friendly;
	int t;


	if (!message || !*message || (friendly = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a valid value (usage: @petfriendly <0-1000>).");
		return false;
	}

	if (sd.status.pet_id > 0 && sd.pd) {
		if (friendly >= 0 && friendly <= 1000) {
			if (friendly != sd.pd->pet.intimate) {
				t = sd.pd->pet.intimate;
				sd.pd->pet.intimate = friendly;
				clif_send_petstatus(sd);
				if (config.pet_status_support) {
					if ((sd.pd->pet.intimate > 0 && t <= 0) ||
					    (sd.pd->pet.intimate <= 0 && t > 0)) {
						if (sd.block_list::prev != NULL)
							status_calc_pc(sd, 0);
						else
							status_calc_pc(sd, 2);
					}
				}
				clif_displaymessage(fd, msg_table[182]); // Pet friendly value changed!
			} else {
				clif_displaymessage(fd, msg_table[183]); // Pet friendly is already the good value.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[184]); // Sorry, but you have no pet.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_pethungry(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int hungry;


	if (!message || !*message || (hungry = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a valid number (usage: @pethungry <0-100>).");
		return false;
	}

	if (sd.status.pet_id > 0 && sd.pd) {
		if (hungry >= 0 && hungry <= 100) {
			if (hungry != sd.pd->pet.hungry) {
				sd.pd->pet.hungry = hungry;
				clif_send_petstatus(sd);
				clif_displaymessage(fd, msg_table[185]); // Pet hungry value changed!
			} else {
				clif_displaymessage(fd, msg_table[186]); // Pet hungry is already the good value.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[184]); // Sorry, but you have no pet.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_petrename(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	if (sd.status.pet_id > 0 && sd.pd) {
		if (sd.pd->pet.rename_flag != 0) {
			sd.pd->pet.rename_flag = 0;
			intif_save_petdata(sd.status.account_id, sd.pd->pet);
			clif_send_petstatus(sd);
			clif_displaymessage(fd, msg_table[187]); // You can now rename your pet.
		} else {
			clif_displaymessage(fd, msg_table[188]); // You can already rename your pet.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[184]); // Sorry, but you have no pet.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_recall(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128];
	char player_name[128]="";
	struct map_session_data *pl_sd = NULL;



	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @recall <char name>).");
		return false;
	}

	memset(player_name, '\0', sizeof player_name);
	if(sscanf(message, "%99[^\n]", player_name) < 1)
		return false;
	if(strncmp(sd.status.name,player_name,24)==0)
		return false;

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can recall only lower or same level
			if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
				clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
				return false;
			}
			if (pl_sd->block_list::m < map_num && maps[pl_sd->block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
				clif_displaymessage(fd, "You are not authorised to warp this player from its actual map.");
				return false;
			}
			pc_setpos(*pl_sd, sd.mapname, sd.block_list::x, sd.block_list::y, 2);
			snprintf(output, sizeof(output), msg_table[46], player_name); // %s recalled!
			clif_displaymessage(fd, output);
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_revive(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128];
	struct map_session_data *pl_sd;


	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @revive <char name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL)
	{
		if (pc_isdead(*pl_sd))
		{
			pl_sd->status.hp = pl_sd->status.max_hp;
			clif_skill_nodamage(sd,sd,ALL_RESURRECTION,4,1);
			pc_setstand(*pl_sd);
			if (config.pc_invincible_time > 0)
				pc_setinvincibletimer(*pl_sd, config.pc_invincible_time);
			clif_updatestatus(*pl_sd, SP_HP);
			clif_updatestatus(*pl_sd, SP_SP);
			clif_resurrection(*pl_sd, 1);
			clif_displaymessage(fd, msg_table[51]); // Character revived.
			return true;
		}
	}
	else
		clif_displaymessage(fd, msg_table[3]); // Character not found.

	return false;
	}

///////////////////////////////////////////////////////////////////////////////
///
/// charchangesex command (usage: charchangesex <player_name>)

bool atcommand_char_change_sex(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128];


	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charchangesex <name>).");
		return false;
}

	// check player name
	if(strlen(player_name) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return false;
	} else if(strlen(player_name) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return false;
	} else {
		chrif_char_ask_name(sd.status.account_id, player_name, 5, 0, 0, 0, 0, 0, 0); // type: 5 - changesex
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// charblock command (usage: charblock <player_name>)
/// This command do a definitiv ban on a player
bool atcommand_char_block(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";


	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charblock/@block <name>).");
		return false;
	}

	// check player name
	if(strlen(player_name) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return false;
	} else if(strlen(player_name) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return false;
	} else {
		chrif_char_ask_name(sd.status.account_id, player_name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// charban command (usage: charban <time> <player_name>)
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
/// <example> @ban +1m-2mn1s-6y test_player
///           this example adds 1 month and 1 second, and substracts 2 minutes and 6 years at the same time.
///
bool atcommand_char_ban(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	char output[128]="";
	char * modif_p;
	int year, month, day, hour, minute, second, value;



	if(!message || !*message || sscanf(message, "%s %99[^\n]", output, player_name) < 2) {
		clif_displaymessage(fd, "Please, enter ban time and a player name (usage: @charban/@ban/@banish/@charbanish <time> <name>).");
		return false;
	}

	modif_p = output;
	year = month = day = hour = minute = second = 0;
	while (modif_p[0] != '\0') {
		value = atoi(modif_p);
		if (value == 0)
			modif_p++;
		else {
			if (modif_p[0] == '-' || modif_p[0] == '+')
				modif_p++;
			while (modif_p[0] >= '0' && modif_p[0] <= '9')
				modif_p++;
			if (modif_p[0] == 's') {
				second = value;
				modif_p++;
			} else if (modif_p[0] == 'm' && modif_p[1] == 'n') {
				minute = value;
				modif_p = modif_p + 2;
			} else if (modif_p[0] == 'h') {
				hour = value;
				modif_p++;
			} else if (modif_p[0] == 'd' || modif_p[0] == 'j') {
				day = value;
				modif_p++;
			} else if (modif_p[0] == 'm') {
				month = value;
				modif_p++;
			} else if (modif_p[0] == 'y' || modif_p[0] == 'a') {
				year = value;
				modif_p++;
			} else if (modif_p[0] != '\0') {
				modif_p++;
			}
		}
	}
	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
		clif_displaymessage(fd, msg_table[85]); // Invalid time for ban command.
		return false;
	}

	// check player name
	if(strlen(player_name) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return false;
	} else if(strlen(player_name) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return false;
	} else {
		chrif_char_ask_name(sd.status.account_id, player_name, 2, year, month, day, hour, minute, second); // type: 2 - ban
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// charunblock command (usage: charunblock <player_name>)

bool atcommand_char_unblock(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";


	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charunblock <player_name>).");
		return false;
	}

	// check player name
	if(strlen(player_name) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return false;
	} else if(strlen(player_name) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return false;
	} else {
		// send answer to login server via char-server
		chrif_char_ask_name(sd.status.account_id, player_name, 3, 0, 0, 0, 0, 0, 0); // type: 3 - unblock
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// charunban command (usage: charunban <player_name>)

bool atcommand_char_unban(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";


	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charunban <player_name>).");
		return false;
	}

	// check player name
	if(strlen(player_name) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return false;
	} else if(strlen(player_name) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return false;
	} else {
		// send answer to login server via char-server
		chrif_char_ask_name(sd.status.account_id, player_name, 4, 0, 0, 0, 0, 0, 0); // type: 4 - unban
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_night(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	if (night_flag != 1) {
		map_night_timer(night_timer_tid, 0, 0, 1);
	} else {
		clif_displaymessage(fd, msg_table[89]); // Sorry, it's already the night. Impossible to execute the command.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_day(int fd, struct map_session_data &sd, const char* command, const char* message)
{


	if (night_flag != 0) {
		map_day_timer(day_timer_tid, 0, 0, 1);
	} else {
		clif_displaymessage(fd, msg_table[90]); // Sorry, it's already the day. Impossible to execute the command.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_doom(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i;
	
	clif_specialeffect(sd,450,2);
	for(i = 0; i < fd_max; ++i) {
		if(session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth && i != (size_t)fd &&
		    sd.isGM() >= pl_sd->isGM()) { // you can doom only lower or same gm level
			pc_damage(*pl_sd, pl_sd->status.hp + 1,NULL);
			clif_displaymessage(pl_sd->fd, msg_table[61]); // The holy messenger has given judgement.
		}
	}
	clif_displaymessage(fd, msg_table[62]); // Judgement was made.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_doommap(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i;
	
	clif_specialeffect(sd,450,3);
	for (i = 0; i < fd_max; ++i) {
		if(session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth && i != (size_t)fd && sd.block_list::m == pl_sd->block_list::m &&
		    sd.isGM() >= pl_sd->isGM()) { // you can doom only lower or same gm level
			pc_damage(*pl_sd, pl_sd->status.hp + 1,NULL);
//			clif_specialeffect(&pl_sd->bl,450,1);
			clif_displaymessage(pl_sd->fd, msg_table[61]); // The holy messenger has given judgement.
		}
	}
	clif_displaymessage(fd, msg_table[62]); // Judgement was made.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
void atcommand_raise_sub(struct map_session_data& sd)
{
	if(sd.state.auth && pc_isdead(sd))
	{
		clif_skill_nodamage(sd,sd,ALL_RESURRECTION,4,1);
		sd.status.hp = sd.status.max_hp;
		sd.status.sp = sd.status.max_sp;
		pc_setstand(sd);
		clif_updatestatus(sd, SP_HP);
		clif_updatestatus(sd, SP_SP);
		clif_resurrection(sd, 1);
		if (config.pc_invincible_time > 0)
			pc_setinvincibletimer(sd, config.pc_invincible_time);
		clif_displaymessage(sd.fd, msg_table[63]); // Mercy has been shown.
	}
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_raise(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i;
	for (i = 0; i < fd_max; ++i)
	{	
		if(session[i] && (pl_sd=(struct map_session_data *)session[i]->user_session) && pl_sd->state.auth && sd.block_list::m == pl_sd->block_list::m)
			atcommand_raise_sub(*pl_sd);
	}
	clif_displaymessage(fd, msg_table[64]); // Mercy has been granted.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_raisemap(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i;
	for (i = 0; i < fd_max; ++i)
	{
		if(session[i] && (pl_sd=(struct map_session_data *)session[i]->user_session) && pl_sd->state.auth && sd.block_list::m == pl_sd->block_list::m)
			atcommand_raise_sub(*pl_sd);
	}
	clif_displaymessage(fd, msg_table[64]); // Mercy has been granted.
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// atcommand_character_baselevel @charbaselvlÇ≈ëŒè€ÉLÉÉÉâÇÃÉåÉxÉãÇè„Ç∞ÇÈ

bool atcommand_character_baselevel(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;
	int level = 0, i;


	if(!message || !*message || sscanf(message, "%d %99[^\n]", &level, player_name) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement and a player name (usage: @charbaselvl <#> <nickname>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can change base level only lower or same gm level

			if (level > 0) {
				if (pl_sd->status.base_level == config.maximum_level) {	// check for max level by Valaris
					clif_displaymessage(fd, msg_table[91]); // Character's base level can't go any higher.
					return true;
				}	// End Addition
				if((size_t)level > config.maximum_level || (size_t)level > (config.maximum_level - pl_sd->status.base_level)) // fix positiv overflow
					level = config.maximum_level - pl_sd->status.base_level;
				for (i = 1; i <= level; ++i)
					pl_sd->status.status_point += (pl_sd->status.base_level + i + 14) / 5;
				pl_sd->status.base_level += level;
				clif_updatestatus(*pl_sd, SP_BASELEVEL);
				clif_updatestatus(*pl_sd, SP_NEXTBASEEXP);
				clif_updatestatus(*pl_sd, SP_STATUSPOINT);
				status_calc_pc(*pl_sd, 0);
				pc_heal(*pl_sd, pl_sd->status.max_hp, pl_sd->status.max_sp);
				clif_misceffect(*pl_sd, 0);
				clif_displaymessage(fd, msg_table[65]); // Character's base level raised.
			} else {
				if (pl_sd->status.base_level == 1) {
					clif_displaymessage(fd, msg_table[193]); // Character's base level can't go any lower.
					return false;
}
				if( config.maximum_level < (size_t)(-level) || pl_sd->status.base_level < (size_t)(1 - level)) // fix negativ overflow
					level = 1 - pl_sd->status.base_level;
				if (pl_sd->status.status_point > 0)
				{
					int sp = pl_sd->status.status_point;
					for (i = 0; i > level; i--)
						sp -= (pl_sd->status.base_level + i + 14) / 5;
					pl_sd->status.status_point = (sp<0) ? 0 : sp;
					clif_updatestatus(*pl_sd, SP_STATUSPOINT);
				} // to add: remove status points from stats
				pl_sd->status.base_level += level;
				clif_updatestatus(*pl_sd, SP_BASELEVEL);
				clif_updatestatus(*pl_sd, SP_NEXTBASEEXP);
				status_calc_pc(*pl_sd, 0);
				clif_displaymessage(fd, msg_table[66]); // Character's base level lowered.
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true; //ê≥èÌèIóπ
}

///////////////////////////////////////////////////////////////////////////////
///
/// atcommand_character_joblevel @charjoblvlÇ≈ëŒè€ÉLÉÉÉâÇÃJobÉåÉxÉãÇè„Ç∞ÇÈ

bool atcommand_character_joblevel(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;
	unsigned short max_level = 50;
	int level = 0;
	//ì]ê∂Ç‚ó{éqÇÃèÍçáÇÃå≥ÇÃêEã∆ÇéZèoÇ∑ÇÈ
	struct pc_base_job pl_s_class;


	if(!message || !*message || sscanf(message, "%d %99[^\n]", &level, player_name) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement and a player name (usage: @charjlvl <#> <nickname>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		pl_s_class = pc_calc_base_job(pl_sd->status.class_);
		if (sd.isGM() >= pl_sd->isGM()) { // you can change job level only lower or same gm level
			if (pl_s_class.job == 0)
				max_level -= 40;
			// super novices can go up to 99 [celest]
			else if (pl_s_class.job == 23)
				max_level += 49;
			else if (pl_sd->status.class_ > 4007 && pl_sd->status.class_ < 4023)
				max_level += 20;

			if (level > 0) {
				if (pl_sd->status.job_level == max_level) {
					clif_displaymessage(fd, msg_table[67]); // Character's job level can't go any higher.
					return false;
				}
				if (pl_sd->status.job_level + level > max_level)
					level = max_level - pl_sd->status.job_level;
				pl_sd->status.job_level += level;
				clif_updatestatus(*pl_sd, SP_JOBLEVEL);
				clif_updatestatus(*pl_sd, SP_NEXTJOBEXP);
				pl_sd->status.skill_point += level;
				clif_updatestatus(*pl_sd, SP_SKILLPOINT);
				status_calc_pc(*pl_sd, 0);
				clif_misceffect(*pl_sd, 1);
				clif_displaymessage(fd, msg_table[68]); // character's job level raised.
			} else {
				if (pl_sd->status.job_level == 1) {
					clif_displaymessage(fd, msg_table[194]); // Character's job level can't go any lower.
					return false;
				}
				if (pl_sd->status.job_level + level < 1)
					level = 1 - pl_sd->status.job_level;
				pl_sd->status.job_level += level;
				clif_updatestatus(*pl_sd, SP_JOBLEVEL);
				clif_updatestatus(*pl_sd, SP_NEXTJOBEXP);
				if (pl_sd->status.skill_point > 0)
				{
					int sp = pl_sd->status.skill_point;
					sp += level;
					pl_sd->status.skill_point = (sp<0) ? 0 : sp;
					clif_updatestatus(*pl_sd, SP_SKILLPOINT);
				} // to add: remove status points from skills
				status_calc_pc(*pl_sd, 0);
				clif_displaymessage(fd, msg_table[69]); // Character's job level lowered.
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_kick(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;


	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @kick <charname>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) // you can kick only lower or same gm level
			clif_GM_kick(sd, *pl_sd, 1);
		else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_kickall(int fd, struct map_session_data &sd, const char* command, const char* message)
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

	clif_displaymessage(fd, msg_table[195]); // All players have been kicked!

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_allskill(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	pc_allskillup(sd); // all skills
	sd.status.skill_point = 0; // 0 skill points
	clif_updatestatus(sd, SP_SKILLPOINT); // update
	clif_displaymessage(fd, msg_table[76]); // You have received all skills.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_questskill(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	short skill_id;


	if (!message || !*message || (skill_id = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number (usage: @questskill <#:0+>).");
		return false;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL_DB) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if (pc_checkskill(sd, skill_id) == 0) {
				pc_skill(sd, skill_id, 1, 0);
				clif_displaymessage(fd, msg_table[70]); // You have learned the skill.
			} else {
				clif_displaymessage(fd, msg_table[196]); // You already have this quest skill.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_table[197]); // This skill number doesn't exist or isn't a quest skill.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[198]); // This skill number doesn't exist.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_charquestskill(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;
	int skill_id = 0;


	if(!message || !*message || sscanf(message, "%d %99[^\n]", &skill_id, player_name) < 2 || skill_id < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number and a player name (usage: @charquestskill <#:0+> <char_name>).");
		return false;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL_DB) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if((pl_sd = map_nick2sd(player_name)) != NULL) {
				if (pc_checkskill(*pl_sd, skill_id) == 0) {
					pc_skill(*pl_sd, skill_id, 1, 0);
					clif_displaymessage(fd, msg_table[199]); // This player has learned the skill.
				} else {
					clif_displaymessage(fd, msg_table[200]); // This player already has this quest skill.
					return false;
				}
			} else {
				clif_displaymessage(fd, msg_table[3]); // Character not found.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_table[197]); // This skill number doesn't exist or isn't a quest skill.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[198]); // This skill number doesn't exist.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_lostskill(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	short skill_id;


	if (!message || !*message || (skill_id = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number (usage: @lostskill <#:0+>).");
		return false;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if (pc_checkskill(sd, skill_id) > 0) {
				sd.status.skill[skill_id].lv = 0;
				sd.status.skill[skill_id].flag = 0;
				clif_skillinfoblock(sd);
				clif_displaymessage(fd, msg_table[71]); // You have forgotten the skill.
			} else {
				clif_displaymessage(fd, msg_table[201]); // You don't have this quest skill.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_table[197]); // This skill number doesn't exist or isn't a quest skill.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[198]); // This skill number doesn't exist.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_charlostskill(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;
	int skill_id = 0;


	if(!message || !*message || sscanf(message, "%d %99[^\n]", &skill_id, player_name) < 2 || skill_id < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number and a player name (usage: @charlostskill <#:0+> <char_name>).");
		return false;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if((pl_sd = map_nick2sd(player_name)) != NULL) {
				if (pc_checkskill(*pl_sd, skill_id) > 0) {
					pl_sd->status.skill[skill_id].lv = 0;
					pl_sd->status.skill[skill_id].flag = 0;
					clif_skillinfoblock(*pl_sd);
					clif_displaymessage(fd, msg_table[202]); // This player has forgotten the skill.
				} else {
					clif_displaymessage(fd, msg_table[203]); // This player doesn't have this quest skill.
					return false;
				}
			} else {
				clif_displaymessage(fd, msg_table[3]); // Character not found.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_table[197]); // This skill number doesn't exist or isn't a quest skill.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[198]); // This skill number doesn't exist.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_spiritball(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int number;


	if (!message || !*message || (number = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a spirit ball number (usage: @spiritball <number: 0-1000>).");
		return false;
	}

	// set max number to avoid server/client crash (500 create big balls of several balls: no visial difference with more)
	if (number > 500)
		number = 500;

	if (number >= 0 && number <= 0x7FFF) {
		if (sd.spiritball != number || number > 499) {
			if (sd.spiritball > 0)
				pc_delspiritball(sd, sd.spiritball, 1);
			sd.spiritball = number;
			clif_spiritball(sd);
			// no message, player can look the difference
			if (number > 1000)
				clif_displaymessage(fd, msg_table[204]); // WARNING: more than 1000 spiritballs can CRASH your server and/or client!
		} else {
			clif_displaymessage(fd, msg_table[205]); // You already have this number of spiritballs.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_party(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char party[128];


	memset(party, '\0', sizeof(party));

	if (!message || !*message || sscanf(message, "%99[^\n]", party) < 1) {
		clif_displaymessage(fd, "Please, enter a party name (usage: @party <party_name>).");
		return false;
	}

	party_create(sd, party, 0, 0);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_guild(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char guild[128];
	int prev;


	memset(guild, '\0', sizeof(guild));

	if (!message || !*message || sscanf(message, "%99[^\n]", guild) < 1) {
		clif_displaymessage(fd, "Please, enter a guild name (usage: @guild <guild_name>).");
		return false;
	}

	prev = config.guild_emperium_check;
	config.guild_emperium_check = 0;
	guild_create(sd, guild);
	config.guild_emperium_check = prev;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_agitstart(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	if (agit_flag == 1) {
		clif_displaymessage(fd, msg_table[73]); // Already it has started siege warfare.
		return false;
	}

	agit_flag = 1;
	guild_agit_start();
	clif_displaymessage(fd, msg_table[72]); // Guild siege warfare start!

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_agitend(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	if (agit_flag == 0) {
		clif_displaymessage(fd, msg_table[75]); // Siege warfare hasn't started yet.
		return false;
	}

	agit_flag = 0;
	guild_agit_end();
	clif_displaymessage(fd, msg_table[74]); // Guild siege warfare end!

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @mapexitÇ≈É}ÉbÉvÉTÅ[ÉoÅ[ÇèIóπÇ≥ÇπÇÈ

bool atcommand_mapexit(int fd, struct map_session_data &sd, const char* command, const char* message)
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
/// idsearch <part_of_name>: revrited by [Yor]

bool atcommand_idsearch(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char item_name[128]="";
	char output[128];
	unsigned int i, match;
	struct item_data *item;


	if (!message || !*message || sscanf(message, "%99s", item_name) < 0) {
		clif_displaymessage(fd, "Please, enter a part of item name (usage: @idsearch <part_of_item_name>).");
		return false;
	}

	snprintf(output, sizeof(output), msg_table[77], item_name); // The reference result of '%s' (name: id):
	clif_displaymessage(fd, output);
	match = 0;
	for(i=0; i < MAX_ITEMS; ++i) {
		if ((item = itemdb_exists(i)) != NULL && strstr(item->jname, item_name) != NULL) {
			match++;
			snprintf(output, sizeof(output), msg_table[78], item->jname, item->nameid); // %s: %d
			clif_displaymessage(fd, output);
		}
	}
	snprintf(output, sizeof(output), msg_table[79], match); // It is %d affair above.
	clif_displaymessage(fd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Character Skill Reset

bool atcommand_charskreset(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;

	char output[128];
	char player_name[128]="";

	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charskreset <charname>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can reset skill points only lower or same gm level
			pc_resetskill(*pl_sd);
			snprintf(output, sizeof(output), msg_table[206], player_name); // '%s' skill points reseted!
			clif_displaymessage(fd, output);
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Character Stat Reset

bool atcommand_charstreset(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char output[128];
	char player_name[128]="";



	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charstreset <charname>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can reset stats points only lower or same gm level
			pc_resetstate(*pl_sd);
			snprintf(output, sizeof(output), msg_table[207], player_name); // '%s' stats points reseted!
			clif_displaymessage(fd, output);
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Character Model by chbrules

bool atcommand_charmodel(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	unsigned int hair_style = 0, hair_color = 0, cloth_color = 0;
	struct map_session_data *pl_sd;
	char output[128];
	char player_name[128];


	if(!message || !*message || sscanf(message, "%d %d %d %99[^\n]", &hair_style, &hair_color, &cloth_color, player_name) < 4 || hair_style < 0 || hair_color < 0 || cloth_color < 0) {
		snprintf(output, sizeof(output), "Please, enter a valid model and a player name (usage: @charmodel <hair ID: %ld-%ld> <hair color: %ld-%ld> <clothes color: %ld-%ld> <name>).",
		        (unsigned long)MIN_HAIR_STYLE, (unsigned long)MAX_HAIR_STYLE, (unsigned long)MIN_HAIR_COLOR, (unsigned long)MAX_HAIR_COLOR, (unsigned long)MIN_CLOTH_COLOR, (unsigned long)MAX_CLOTH_COLOR);
		clif_displaymessage(fd, output);
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE &&
		    hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR &&
		    cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {

			if (cloth_color != 0 &&
			    pl_sd->status.sex == 1 &&
			    (pl_sd->status.class_ == 12 ||  pl_sd->status.class_ == 17)) {
				clif_displaymessage(fd, msg_table[35]); // You can't use this command with this class_.
				return false;
			} else {
				pc_changelook(*pl_sd, LOOK_HAIR, hair_style);
				pc_changelook(*pl_sd, LOOK_HAIR_COLOR, hair_color);
				pc_changelook(*pl_sd, LOOK_CLOTHES_COLOR, cloth_color);
				clif_displaymessage(fd, msg_table[36]); // Appearence changed.
			}
		} else {
			clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Character Skill Point (Rewritten by [Yor])

bool atcommand_charskpoint(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;
	int new_skill_point;
	int point = 0;


	if(!message || !*message || sscanf(message, "%d %99[^\n]", &point, player_name) < 2 || point == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: @charskpoint <amount> <name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		new_skill_point = (int)pl_sd->status.skill_point + point;
		if (point > 0 && (point > 0x7FFF || new_skill_point > 0x7FFF)) // fix positiv overflow
			new_skill_point = 0x7FFF;
		else if (point < 0 && (point < -0x7FFF || new_skill_point < 0)) // fix negativ overflow
			new_skill_point = 0;
		if (new_skill_point != (int)pl_sd->status.skill_point) {
			pl_sd->status.skill_point = new_skill_point;
			clif_updatestatus(*pl_sd, SP_SKILLPOINT);
			clif_displaymessage(fd, msg_table[209]); // Character's number of skill points changed!
		} else {
			if (point < 0)
				clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Character Status Point (rewritten by [Yor])

bool atcommand_charstpoint(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;
	int new_status_point;
	int point = 0;


	if(!message || !*message || sscanf(message, "%d %99[^\n]", &point, player_name) < 2 || point == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: @charstpoint <amount> <name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		new_status_point = (int)pl_sd->status.status_point + point;
		if (point > 0 && (point > 0x7FFF || new_status_point > 0x7FFF)) // fix positiv overflow
			new_status_point = 0x7FFF;
		else if (point < 0 && (point < -0x7FFF || new_status_point < 0)) // fix negativ overflow
			new_status_point = 0;
		if (new_status_point != (int)pl_sd->status.status_point) {
			pl_sd->status.status_point = new_status_point;
			clif_updatestatus(*pl_sd, SP_STATUSPOINT);
			clif_displaymessage(fd, msg_table[210]); // Character's number of status points changed!
		} else {
			if (point < 0)
				clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Recall All Characters Online To Your Location

bool atcommand_recallall(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char output[128];
	size_t i, count;



	if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return false;
	}

	count = 0;
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth && sd.status.account_id != pl_sd->status.account_id &&
		    sd.isGM() >= pl_sd->isGM()) { // you can recall only lower or same level
			if (pl_sd->block_list::m < map_num && maps[pl_sd->block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM())
				count++;
			else
				pc_setpos(*pl_sd, sd.mapname, sd.block_list::x, sd.block_list::y, 2);
		}
	}

	clif_displaymessage(fd, msg_table[92]); // All characters recalled!
	if (count) {
		snprintf(output, sizeof(output), "Because you are not authorised to warp from some maps, %ld player(s) have not been recalled.", (unsigned long)(count));
		clif_displaymessage(fd, output);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Recall online characters of a guild to your location

bool atcommand_guildrecall(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	size_t i, count;
	char guild_name[128]="";
	char output[128];
	struct guild *g;



	if (!message || !*message || sscanf(message, "%99[^\n]", guild_name) < 1) {
		clif_displaymessage(fd, "Please, enter a guild name/id (usage: @guildrecall <guild_name/id>).");
		return false;
	}

	if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return false;
	}

	if ((g = guild_searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild_search(atoi(message))) != NULL) {
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
		snprintf(output, sizeof(output), msg_table[93], g->name); // All online characters of the %s guild are near you.
		clif_displaymessage(fd, output);
		if (count) {
			snprintf(output, sizeof(output), "Because you are not authorised to warp from some maps, %ld player(s) have not been recalled.", (unsigned long)(count));
			clif_displaymessage(fd, output);
		}
	} else {
		clif_displaymessage(fd, msg_table[94]); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Recall online characters of a party to your location

bool atcommand_partyrecall(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	size_t i, count;
	struct map_session_data *pl_sd;
	char party_name[128]="";
	char output[128];
	struct party *p;



	if (!message || !*message || sscanf(message, "%99[^\n]", party_name) < 1) {
		clif_displaymessage(fd, "Please, enter a party name/id (usage: @partyrecall <party_name/id>).");
		return false;
	}

	if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return false;
	}

	if ((p = party_searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party_search(atoi(message))) != NULL) {
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
		snprintf(output, sizeof(output), msg_table[95], p->name); // All online characters of the %s party are near you.
		clif_displaymessage(fd, output);
		if (count) {
			snprintf(output, sizeof(output), "Because you are not authorised to warp from some maps, %ld player(s) have not been recalled.", (unsigned long)(count));
			clif_displaymessage(fd, output);
		}
	} else {
		clif_displaymessage(fd, msg_table[96]); // Incorrect name or ID, or no one from the party is online.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_reloaditemdb(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	itemdb_reload();
	clif_displaymessage(fd, msg_table[97]); // Item database reloaded.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_reloadmobdb(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	mob_reload();
	do_final_pet();
	read_petdb();
	clif_displaymessage(fd, msg_table[98]); // Monster database reloaded.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_reloadskilldb(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	skill_reload();
	clif_displaymessage(fd, msg_table[99]); // Skill database reloaded.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @reloadatcommand
///  atcommand_athena.conf ÇÃÉäÉçÅ[Éh

bool atcommand_reloadatcommand(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	atcommand_config_read(ATCOMMAND_CONF_FILENAME);
	clif_displaymessage(fd, msg_table[254]);
	return true;
}
/*==========================================
 * @reloadbattleconf
 *   battle_athena.conf ÇÃÉäÉçÅ[Éh
 *------------------------------------------
 */
bool atcommand_reloadbattleconf(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	config_read(BATTLE_CONF_FILENAME);
	clif_displaymessage(fd, msg_table[255]);
	return true;
}
///////////////////////////////////////////////////////////////////////////////
///
/// @reloadstatusdb
///  job_db1.txt job_db2.txt job_db2-2.txt 
///  refine_db.txt size_fix.txt
///  ÇÃÉäÉçÅ[Éh

bool atcommand_reloadstatusdb(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	status_readdb();
	clif_displaymessage(fd, msg_table[256]);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @reloadpcdb
///  exp.txt skill_tree.txt attr_fix.txt 
///  ÇÃÉäÉçÅ[Éh
bool atcommand_reloadpcdb(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	pc_readdb();
	clif_displaymessage(fd, msg_table[257]);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_reloadscript(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	atcommand_broadcast( fd, sd, "@broadcast", "eAthena Server is Rehashing..." );
	atcommand_broadcast( fd, sd, "@broadcast", "You will feel a bit of lag at this point !" );
	atcommand_broadcast( fd, sd, "@broadcast", "Reloading NPCs..." );

	flush_fifos();

	do_init_script();
	npc_reload();
	npc_event_do_oninit();

	clif_displaymessage(fd, msg_table[128]); // Scripts reloaded.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_reloadgmdb( // by [Yor]
	int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	chrif_reloadGMdb();

	clif_displaymessage(fd, msg_table[101]); // Login-server asked to reload GM accounts and their level.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @mapinfo <map name> [0-3] by MC_Cameri
/// => Shows information about the map [map name]
/// 0 = no additional information
/// 1 = Show users in that map and their location
/// 2 = Shows NPCs in that map
/// 3 = Shows the shops/chats in that map (not implemented)
///
bool atcommand_mapinfo(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	struct npc_data *nd = NULL;
	chat_data *cd = NULL;
	char direction[12]="";
	char mapname[128]="", *ip;
	char output[128];
	int m_id, chat_num, list = 0;
	size_t i;


	sscanf(message, "%d %99[^\n]", &list, mapname);

	if (list < 0 || list > 3) {
		clif_displaymessage(fd, "Please, enter at least a valid list number (usage: @mapinfo <0-3> [map]).");
		return false;
	}

	if(mapname[0] == '\0')
		memcpy(mapname, sd.mapname, 24);
	ip = strchr(mapname, '.');
		if(ip) *ip=0;

	if((m_id = map_mapname2mapid(mapname)) < 0) {
		clif_displaymessage(fd, msg_table[1]); // Map not found.
		return false;
	}

	clif_displaymessage(fd, "------ Map Info ------");
	chat_num = 0;
	for (i = 0; i < fd_max; ++i) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
		    (cd = (chat_data*)map_id2bl(pl_sd->chatID)))
		{
			chat_num++;
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
		for (i = 0; i < fd_max; ++i) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
			    (cd = (chat_data*)map_id2bl(pl_sd->chatID)) &&
			    strcmp(pl_sd->mapname, mapname) == 0 &&
			    cd->usersd[0] == pl_sd) {
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
		clif_displaymessage(fd, "Please, enter at least a valid list number (usage: @mapinfo <0-3> [map]).");
		return false;
		break;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_mount_peco(int fd, struct map_session_data &sd, const char* command, const char* message)
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
			clif_displaymessage(fd, msg_table[102]); // Mounted Peco.
		} else {
			clif_displaymessage(fd, msg_table[213]); // You can not mount a peco with your job.
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
		clif_displaymessage(fd, msg_table[214]); // Unmounted Peco.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_char_mount_peco(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;


	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charmountpeco <char_name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (!pc_isriding(*pl_sd)) { // if actually no peco
			if (pl_sd->status.class_ == 7 || pl_sd->status.class_ == 14 || pl_sd->status.class_ == 4008 || pl_sd->status.class_ == 4015 || pl_sd->status.class_ == 4030 || pl_sd->status.class_ == 4036 || pl_sd->status.class_ == 4037 || pl_sd->status.class_ == 4044) {
				if (pl_sd->status.class_ == 7)
					pl_sd->status.class_ = pl_sd->view_class = 13;
				else if (pl_sd->status.class_ == 14)
					pl_sd->status.class_ = pl_sd->view_class = 21;
				else if (pl_sd->status.class_ == 4008)
					pl_sd->status.class_ = pl_sd->view_class = 4014;
				else if (pl_sd->status.class_ == 4015)
					pl_sd->status.class_ = pl_sd->view_class = 4022;
				else if (pl_sd->status.class_ == 4030) //baby Knight
					pl_sd->status.class_ = pl_sd->view_class = 4036;
				else if (pl_sd->status.class_ == 4037) //baby Crusader
					pl_sd->status.class_ = pl_sd->view_class = 4044;
				pc_setoption(*pl_sd, pl_sd->status.option | 0x0020);
				clif_displaymessage(fd, msg_table[216]); // Now, this player mounts a peco.
			} else {
				clif_displaymessage(fd, msg_table[217]); // This player can not mount a peco with his/her job.
				return false;
			}
		} else {
			if (pl_sd->status.class_ == 13)
				pl_sd->status.class_ = pl_sd->view_class = 7;
			else if (pl_sd->status.class_ == 21)
				pl_sd->status.class_ = pl_sd->view_class = 14;
			else if (pl_sd->status.class_ == 4014)
				pl_sd->status.class_ = pl_sd->view_class = 4008;
			else if (pl_sd->status.class_ == 4022)
				pl_sd->status.class_ = pl_sd->view_class = 4015;
			else if (pl_sd->status.class_ == 4036) //baby Knight
				pl_sd->status.class_ = pl_sd->view_class = 4030;
			else if (pl_sd->status.class_ == 4044) //baby Crusader
				pl_sd->status.class_ = pl_sd->view_class = 4037;
			pc_setoption(*pl_sd, pl_sd->status.option & ~0x0020);
			clif_displaymessage(fd, msg_table[218]); // Now, this player has not more peco.
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Spy Commands by Syrus22

bool atcommand_guildspy(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char guild_name[128]="";
	char output[128];
	struct guild *g;


	if (!message || !*message || sscanf(message, "%99[^\n]", guild_name) < 1) {
		clif_displaymessage(fd, "Please, enter a guild name/id (usage: @guildspy <guild_name/id>).");
		return false;
	}

	if ((g = guild_searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild_search(atoi(message))) != NULL) {
		if (sd.guildspy == g->guild_id) {
			sd.guildspy = 0;
			snprintf(output, sizeof(output), msg_table[103], g->name); // No longer spying on the %s guild.
			clif_displaymessage(fd, output);
		} else {
			sd.guildspy = g->guild_id;
			snprintf(output, sizeof(output), msg_table[104], g->name); // Spying on the %s guild.
			clif_displaymessage(fd, output);
		}
	} else {
		clif_displaymessage(fd, msg_table[94]); // Incorrect name/ID, or no one from the guild is online.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_partyspy(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char party_name[128]="";
	char output[128];
	struct party *p;



	if (!message || !*message || sscanf(message, "%99[^\n]", party_name) < 1) {
		clif_displaymessage(fd, "Please, enter a party name/id (usage: @partyspy <party_name/id>).");
		return false;
	}

	if ((p = party_searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party_search(atoi(message))) != NULL) {
		if (sd.partyspy == p->party_id) {
			sd.partyspy = 0;
			snprintf(output, sizeof(output), msg_table[105], p->name); // No longer spying on the %s party.
			clif_displaymessage(fd, output);
		} else {
			sd.partyspy = p->party_id;
			snprintf(output, sizeof(output), msg_table[106], p->name); // Spying on the %s party.
			clif_displaymessage(fd, output);
		}
	} else {
		clif_displaymessage(fd, msg_table[96]); // Incorrect name or ID, or no one from the party is online.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @repairall [Valaris]

bool atcommand_repairall(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int count, i;


	count = 0;
	for (i = 0; i < MAX_INVENTORY; ++i) {
		if (sd.status.inventory[i].nameid && sd.status.inventory[i].attribute == 1) {
			sd.status.inventory[i].attribute = 0;
			clif_produceeffect(sd, sd.status.inventory[i].nameid, 0);
			count++;
		}
	}

	if (count > 0) {
		clif_misceffect(sd, 3);
		clif_equiplist(sd);
		clif_displaymessage(fd, msg_table[107]); // All items have been repaired.
	} else {
		clif_displaymessage(fd, msg_table[108]); // No item need to be repaired.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Removed @nuke for now in favor of alchemist marine sphere skill [Valaris]
bool atcommand_nuke(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;


	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @nuke <char name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can kill only lower or same GM level
			skill_castend_damage_id(pl_sd, pl_sd, NPC_SELFDESTRUCTION, 99, gettick(), 0);
			clif_displaymessage(fd, msg_table[109]); // Player has been nuked!
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_shownpc(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char NPCname[128]="";

	if (!message || !*message || sscanf(message, "%99[^\n]", NPCname) < 1) {
		clif_displaymessage(fd, "Please, enter a NPC name (usage: @enablenpc <NPC_name>).");
		return false;
	}

	if (npc_name2id(NPCname) != NULL) {
		npc_enable(NPCname, 1);
		clif_displaymessage(fd, msg_table[110]); // Npc Enabled.
	} else {
		clif_displaymessage(fd, msg_table[111]); // This NPC doesn't exist.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_hidenpc(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char NPCname[128];


	memset(NPCname, '\0', sizeof(NPCname));

	if (!message || !*message || sscanf(message, "%99[^\n]", NPCname) < 1) {
		clif_displaymessage(fd, "Please, enter a NPC name (usage: @npcoff <NPC_name>).");
		return false;
	}

	if (npc_name2id(NPCname) != NULL) {
		npc_enable(NPCname, 0);
		clif_displaymessage(fd, msg_table[112]); // Npc Disabled.
	} else {
		clif_displaymessage(fd, msg_table[111]); // This NPC doesn't exist.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_loadnpc(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	FILE *fp;

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a script file name (usage: @loadnpc <file name>).");
		return false;
	}

	// check if script file exists
	if ((fp = basics::safefopen(message, "r")) == NULL) {
		clif_displaymessage(fd, msg_table[261]);
		return false;
	}
	fclose(fp);

	// add to list of script sources and run it
	npc_addsrcfile(message);
	npc_parsesrcfile(message);

	clif_displaymessage(fd, msg_table[262]);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_unloadnpc(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct npc_data *nd;
	char NPCname[100];


	memset(NPCname, '\0', sizeof(NPCname));

	if (!message || !*message || sscanf(message, "%99[^\n]", NPCname) < 1) {
		clif_displaymessage(fd, "Please, enter a NPC name (usage: @npcoff <NPC_name>).");
		return false;
	}

	if((nd = npc_name2id(NPCname)) != NULL)
	{
		npc_remove_map(nd);
		clif_displaymessage(fd, msg_table[112]); // Npc Disabled.
	} else {
		clif_displaymessage(fd, msg_table[111]); // This NPC doesn't exist.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// time in txt for time command (by [Yor])
/// !! not threadsave
char * txt_time(unsigned long duration)
{
	unsigned long days, hours, minutes, seconds;
	char temp[256];
	static char temp1[256];

	memset(temp, '\0', sizeof(temp));
	memset(temp1, '\0', sizeof(temp1));

	if (duration < 0)
		duration = 0;

	days = duration / (60 * 60 * 24);
	duration = duration - (60 * 60 * 24 * days);
	hours = duration / (60 * 60);
	duration = duration - (60 * 60 * hours);
	minutes = duration / 60;
	seconds = duration - (60 * minutes);

	if (days < 2)
		snprintf(temp, sizeof(temp), msg_table[219], days); // %d day
	else
		snprintf(temp, sizeof(temp), msg_table[220], days); // %d days
	if (hours < 2)
		snprintf(temp1, sizeof(temp1),msg_table[221], temp, hours); // %s %d hour
	else
		snprintf(temp1, sizeof(temp1), msg_table[222], temp, hours); // %s %d hours
	if (minutes < 2)
		snprintf(temp, sizeof(temp), msg_table[223], temp1, minutes); // %s %d minute
	else
		snprintf(temp, sizeof(temp), msg_table[224], temp1, minutes); // %s %d minutes
	if (seconds < 2)
		snprintf(temp1, sizeof(temp1), msg_table[225], temp, seconds); // %s and %d second
	else
		snprintf(temp1, sizeof(temp1), msg_table[226], temp, seconds); // %s and %d seconds

	return temp1;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @time/@date/@server_date/@serverdate/@server_time/@servertime: Display the date/time of the server (by [Yor]
/// Calculation management of GM modification (@day/@night GM commands) is done
///
bool atcommand_servertime(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct TimerData * timer_data;
	struct TimerData * timer_data2;
	time_t time_server;  // variable for number of seconds (used with time() function)
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	char temp[256]="";

	time(&time_server);  // get time in seconds since 1/1/1970
	datetime = localtime(&time_server); // convert seconds in structure
	// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
	strftime(temp, sizeof(temp)-1, msg_table[230], datetime); // Server time (normal time): %A, %B %d %Y %X.
	clif_displaymessage(fd, temp);

	if (config.night_duration == 0 && config.day_duration == 0) {
		if (night_flag == 0)
			clif_displaymessage(fd, msg_table[231]); // Game time: The game is in permanent daylight.
		else
			clif_displaymessage(fd, msg_table[232]); // Game time: The game is in permanent night.
	} else if (config.night_duration == 0)
		if (night_flag == 1) { // we start with night
			timer_data = get_timer(day_timer_tid);
			snprintf(temp, sizeof(temp), msg_table[233], txt_time((timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in night for %s.
			clif_displaymessage(fd, temp);
			clif_displaymessage(fd, msg_table[234]); // Game time: After, the game will be in permanent daylight.
		} else
			clif_displaymessage(fd, msg_table[231]); // Game time: The game is in permanent daylight.
	else if (config.day_duration == 0)
		if (night_flag == 0) { // we start with day
			timer_data = get_timer(night_timer_tid);
			snprintf(temp, sizeof(temp), msg_table[235], txt_time((timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in daylight for %s.
			clif_displaymessage(fd, temp);
			clif_displaymessage(fd, msg_table[236]); // Game time: After, the game will be in permanent night.
		} else
			clif_displaymessage(fd, msg_table[232]); // Game time: The game is in permanent night.
	else {
		if (night_flag == 0) {
			timer_data = get_timer(night_timer_tid);
			timer_data2 = get_timer(day_timer_tid);
			snprintf(temp, sizeof(temp), msg_table[235], txt_time((timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in daylight for %s.
			clif_displaymessage(fd, temp);
			if (timer_data->tick > timer_data2->tick)
				snprintf(temp, sizeof(temp), msg_table[237], txt_time((timer_data->interval - (uint32)abs((long)(timer_data->tick - timer_data2->tick))) / 1000)); // Game time: After, the game will be in night for %s.
			else
				snprintf(temp, sizeof(temp), msg_table[237], txt_time((uint32)abs((long)(timer_data->tick - timer_data2->tick)) / 1000)); // Game time: After, the game will be in night for %s.
			clif_displaymessage(fd, temp);
			snprintf(temp, sizeof(temp), msg_table[238], txt_time(timer_data->interval / 1000)); // Game time: A day cycle has a normal duration of %s.
			clif_displaymessage(fd, temp);
		} else {
			timer_data = get_timer(day_timer_tid);
			timer_data2 = get_timer(night_timer_tid);
			snprintf(temp, sizeof(temp), msg_table[233], txt_time((timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in night for %s.
			clif_displaymessage(fd, temp);
			if (timer_data->tick > timer_data2->tick)
				snprintf(temp, sizeof(temp), msg_table[239], txt_time((timer_data->interval - (uint32)abs((long)(timer_data->tick - timer_data2->tick))) / 1000)); // Game time: After, the game will be in daylight for %s.
			else
				snprintf(temp, sizeof(temp), msg_table[239], txt_time((uint32)abs((long)(timer_data->tick - timer_data2->tick)) / 1000)); // Game time: After, the game will be in daylight for %s.
			clif_displaymessage(fd, temp);
			snprintf(temp, sizeof(temp), msg_table[238], txt_time(timer_data->interval / 1000)); // Game time: A day cycle has a normal duration of %s.
			clif_displaymessage(fd, temp);
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @chardelitem <item_name_or_ID> <quantity> <player> (by [Yor]
/// removes <quantity> item from a character
/// item can be equiped or not.
/// Inspired from a old command created by RoVeRT
///
bool atcommand_chardelitem(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player_name[128]="";
	char item_name[128]="";
	char output[128];
	int i, number = 0, item_id, item_position, count;
	struct item_data *item_data;


	if(!message || !*message || sscanf(message, "%s %d %99[^\n]", item_name, &number, player_name) < 3 || number < 1) {
		clif_displaymessage(fd, "Please, enter an item name/id, a quantity and a player name (usage: @chardelitem <item_name_or_ID> <quantity> <player>).");
		return false;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id > 500) {
		if((pl_sd = map_nick2sd(player_name)) != NULL) {
			if (sd.isGM() >= pl_sd->isGM()) { // you can kill only lower or same level
				item_position = pc_search_inventory(*pl_sd, item_id);
				if (item_position >= 0) {
					count = 0;
					for(i = 0; i < number && item_position >= 0; ++i) {
						pc_delitem(*pl_sd, item_position, 1, 0);
						count++;
						item_position = pc_search_inventory(*pl_sd, item_id); // for next loop
					}
					snprintf(output, sizeof(output), msg_table[113], count); // %d item(s) removed by a GM.
					clif_displaymessage(pl_sd->fd, output);
					if (number == count)
						snprintf(output, sizeof(output), msg_table[114], count); // %d item(s) removed from the player.
					else
						snprintf(output, sizeof(output), msg_table[115], count, count, number); // %d item(s) removed. Player had only %d on %d items.
					clif_displaymessage(fd, output);
				} else {
					clif_displaymessage(fd, msg_table[116]); // Character does not have the item.
					return false;
				}
			} else {
				clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_table[3]); // Character not found.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[19]); // Invalid item ID or name.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @jail <char_name> by [Yor]
/// Special warp! No check with nowarp and nowarpto flag
///
bool atcommand_jail(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;
	int x, y;


	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @jail <char_name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can jail only lower or same GM
			switch(rand() % 2) {
			case 0:
				x = 24;
				y = 75;
				break;
			default:
				x = 49;
				y = 75;
				break;
			}
			if( pc_setpos(*pl_sd, "sec_pri.gat", x, y, 3) ) {
				pc_setsavepoint(*pl_sd, "sec_pri.gat", x, y); // Save Char Respawn Point in the jail room [Lupus]
				clif_displaymessage(pl_sd->fd, msg_table[117]); // GM has send you in jails.
				clif_displaymessage(fd, msg_table[118]); // Player warped in jails.
			} else {
				clif_displaymessage(fd, msg_table[1]); // Map not found.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @unjail/@discharge <char_name> by [Yor]
/// Special warp! No check with nowarp and nowarpto flag
///
bool atcommand_unjail(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data *pl_sd;

	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @unjail/@discharge <char_name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can jail only lower or same GM
			if (pl_sd->block_list::m != map_mapname2mapid("sec_pri.gat")) {
				clif_displaymessage(fd, msg_table[119]); // This player is not in jails.
				return false;
			} else if (pc_setpos(*pl_sd, "prontera.gat", 0, 0, 3) == 0) { //old coords: 156,191
				pc_setsavepoint(*pl_sd, "prontera.gat", 0, 0); // Save char respawn point in Prontera
				clif_displaymessage(pl_sd->fd, msg_table[120]); // GM has discharge you.
				clif_displaymessage(fd, msg_table[121]); // Player warped to Prontera.
			} else {
				clif_displaymessage(fd, msg_table[1]); // Map not found.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @disguise <mob_id> by [Valaris] (simplified by [Yor])
///
bool atcommand_disguise(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	unsigned short mob_id = 0;
	
	if (!message || !*message)
	{
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: @disguise <monster_name_or_monster_ID>).");
		return false;
	}

	mob_id = mobdb_searchname(message);
	if( mob_id == 0 ) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(message);

	if( (mob_id >=  46 && mob_id <= 125) || // NPC
//		(mob_id >= 700 && mob_id <= 718) || (mob_id >= 721 && mob_id <= 755) || (mob_id >= 757 && mob_id <= 811) || (mob_id >= 813 && mob_id <= 858) || // NPC
		(mob_id >= 700 && mob_id <= 858) || // NPC
	    (mob_id > 1000 && mob_id < 1582)) { // monsters

		sd.stop_walking(0);
		clif_clearchar(sd, 0);
		sd.disguise_id = mob_id;
		clif_changeoption(sd);
		clif_spawnpc(sd);
		clif_displaymessage(fd, msg_table[122]); // Disguise applied.
	} else {
		clif_displaymessage(fd, msg_table[123]); // Monster/NPC name/id hasn't been found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// DisguiseAll
///
bool atcommand_disguiseall(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	unsigned short mob_id=0;
	size_t i=0;
	struct map_session_data *pl_sd;


	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: @disguiseall <monster_name_or_monster_ID>).");
		return false;
	}

	if ((mob_id = mobdb_searchname(message)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(message);

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
		clif_displaymessage(fd, msg_table[122]); // Disguise applied.
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @undisguise by [Yor]
///
bool atcommand_undisguise(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	if(sd.disguise_id)
	{
		sd.stop_walking(0);
		clif_clearchar(sd, 0);
		sd.disguise_id = 0;
		clif_changeoption(sd);
		clif_spawnpc(sd);
		clif_displaymessage(fd, msg_table[124]); // Undisguise applied.
	} else {
		clif_displaymessage(fd, msg_table[125]); // You're not disguised.
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// UndisguiseAll
///
bool atcommand_undisguiseall(int fd, struct map_session_data &sd, const char* command, const char* message)
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
	clif_displaymessage(fd, msg_table[124]); // Undisguise applied.

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @broadcast by [Valaris]
///
bool atcommand_broadcast(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128];


	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a message (usage: @broadcast <message>).");
		return false;
	}

	size_t sz=1+snprintf(output, sizeof(output), "%s : %s", sd.status.name, message);
	intif_GMmessage(output, sz,0);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @localbroadcast by [Valaris]
///
bool atcommand_localbroadcast(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128];


	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a message (usage: @localbroadcast <message>).");
		return false;
	}

	size_t sz=1+snprintf(output, sizeof(output), "%s : %s", sd.status.name, message);
	clif_GMmessage(&sd, output, sz, 1); // 1: ALL_SAMEMAP

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @chardisguise <mob_id> <character> by Kalaspuff (based off Valaris' and Yor's work)
///
bool atcommand_chardisguise(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	unsigned short mob_id;
	char player_name[128]="";
	char mob_name[128]="";
	struct map_session_data* pl_sd;



	if(!message || !*message || sscanf(message, "%s %99[^\n]", mob_name, player_name) < 2) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id and a player name (usage: @chardisguise <monster_name_or_monster_ID> <char name>).");
		return false;
	}

	if ((mob_id = mobdb_searchname(mob_name)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(mob_name);

	if((pl_sd = map_nick2sd(player_name)) != NULL)
	{
		if (sd.isGM() >= pl_sd->isGM())
		{	// you can disguise only lower or same level
			if ((mob_id >=  46 && mob_id <= 125) || (mob_id >= 700 && mob_id <= 718) || // NPC
			    (mob_id >= 721 && mob_id <= 755) || (mob_id >= 757 && mob_id <= 811) || // NPC
			    (mob_id >= 813 && mob_id <= 834) || // NPC
			    (mob_id > 1000 && mob_id < 1521))
			{	// monsters
				pl_sd->stop_walking(0);
				clif_clearchar(*pl_sd, 0);
				pl_sd->disguise_id = mob_id;
				clif_changeoption(*pl_sd);
				clif_spawnpc(*pl_sd);
				clif_displaymessage(fd, msg_table[140]); // Character's disguise applied.
			} else {
				clif_displaymessage(fd, msg_table[123]); // Monster/NPC name/id hasn't been found.
				return false;
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @charundisguise <character> by Kalaspuff (based off Yor's work)
///
bool atcommand_charundisguise(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	struct map_session_data* pl_sd;



	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charundisguise <char name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL)
	{
		if (sd.isGM() >= pl_sd->isGM())
		{ // you can undisguise only lower or same level
			if(pl_sd->disguise_id)
			{
				pl_sd->stop_walking(0);
				clif_clearchar(*pl_sd, 0);
				pl_sd->disguise_id = 0;
				clif_changeoption(*pl_sd);
				clif_spawnpc(*pl_sd);
				clif_displaymessage(fd, msg_table[141]); // Character's undisguise applied.
			}
			else
			{
				clif_displaymessage(fd, msg_table[142]); // Character is not disguised.
				return false;
			}
		}
		else
		{
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @email <actual@email> <new@email> by [Yor]
///
bool atcommand_email(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char actual_email[128]="";
	char new_email[128]="";

	if (!message || !*message || sscanf(message, "%99s %99s", actual_email, new_email) < 2) {
		clif_displaymessage(fd, "Please enter 2 emails (usage: @email <actual@email> <new@email>).");
		return false;
	}

	if( !email_check(actual_email) ) {
		clif_displaymessage(fd, msg_table[144]); // Invalid actual email. If you have default e-mail, give a@a.com.
		return false;
	} else if( !email_check(new_email) ) {
		clif_displaymessage(fd, msg_table[145]); // Invalid new email. Please enter a real e-mail.
		return false;
	} else if(strcasecmp(new_email, "a@a.com") == 0) {
		clif_displaymessage(fd, msg_table[146]); // New email must be a real e-mail.
		return false;
	} else if(strcasecmp(actual_email, new_email) == 0) {
		clif_displaymessage(fd, msg_table[147]); // New email must be different of the actual e-mail.
		return false;
	} else {
		chrif_changeemail(sd.status.account_id, actual_email, new_email);
		clif_displaymessage(fd, msg_table[148]); // Information sended to login-server via char-server.
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @effect
///
bool atcommand_effect(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int type = 0, flag = 0;
	size_t i;


	if (!message || !*message || sscanf(message, "%d %d", &type,&flag) < 2) {
		clif_displaymessage(fd, "Please, enter at least a option (usage: @effect <type+>).");
		return false;
	}
	if(flag <=0){
		clif_specialeffect(sd, type, flag);
		clif_displaymessage(fd, msg_table[229]); // Your effect has changed.
	}
	else{
		for (i = 0; i < fd_max; ++i) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->user_session) && pl_sd->state.auth) {
				clif_specialeffect(*pl_sd, type, flag);
				clif_displaymessage(pl_sd->fd, msg_table[229]); // Your effect has changed.
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @charstoragelist <character>: Displays the items list of a player's storage.
///
bool atcommand_character_storage_list(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct pc_storage *stor;
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	size_t i, j, count, counter, counter2;
	char character[128]="";
	char output[256];
	char outputtmp[256];
	

	if(!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charitemlist <char name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(character)) != NULL) {
		if(sd.isGM() >= pl_sd->isGM()) { // you can look items only lower or same level
			if((stor = account2storage2(pl_sd->status.account_id)) != NULL) {
				counter = 0;
				count = 0;
				for (i = 0; i < MAX_STORAGE; ++i) {
					if(stor->storage[i].nameid > 0 && (item_data = itemdb_exists(stor->storage[i].nameid)) != NULL) {
						counter = counter + stor->storage[i].amount;
						count++;
						if(count == 1) {
							snprintf(output, sizeof(output), "------ Storage items list of '%s' ------", pl_sd->status.name);
							clif_displaymessage(fd, output);
						}
						if(stor->storage[i].refine)
							snprintf(output, sizeof(output), "%d %s %+d (%s %+d, id: %d)", stor->storage[i].amount, item_data->name, stor->storage[i].refine, item_data->jname, stor->storage[i].refine, stor->storage[i].nameid);
						else
							snprintf(output, sizeof(output), "%d %s (%s, id: %d)", stor->storage[i].amount, item_data->name, item_data->jname, stor->storage[i].nameid);
						clif_displaymessage(fd, output);
						*output = '\0';
						counter2 = 0;
						for (j = 0; j < item_data->flag.slot; ++j) {
							if(stor->storage[i].card[j]) {
								if((item_temp = itemdb_exists(stor->storage[i].card[j])) != NULL) {
									if( output[0] == '\0')
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
				if(count == 0)
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
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @charcartlist <character>: Displays the items list of a player's cart.
///
bool atcommand_character_cart_list(int fd, struct map_session_data &sd, const char* command, const char* message)
{

	char player_name[128]="";
	char output[128];
	char outputtmp[256];
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	size_t i, j, count, counter, counter2;



	if(!message || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charitemlist <char name>).");
		return false;
	}

	if((pl_sd = map_nick2sd(player_name)) != NULL) {
		if (sd.isGM() >= pl_sd->isGM()) { // you can look items only lower or same level
			counter = 0;
			count = 0;
			for (i = 0; i < MAX_CART; ++i) {
				if (pl_sd->status.cart[i].nameid > 0 && (item_data = itemdb_exists(pl_sd->status.cart[i].nameid)) != NULL) {
					counter = counter + pl_sd->status.cart[i].amount;
					count++;
					if (count == 1) {
						snprintf(output, sizeof(output), "------ Cart items list of '%s' ------", pl_sd->status.name);
						clif_displaymessage(fd, output);
					}
					if (pl_sd->status.cart[i].refine)
						snprintf(output, sizeof(output), "%d %s %+d (%s %+d, id: %d)", pl_sd->status.cart[i].amount, item_data->name, pl_sd->status.cart[i].refine, item_data->jname, pl_sd->status.cart[i].refine, pl_sd->status.cart[i].nameid);
					else
						snprintf(output, sizeof(output), "%d %s (%s, id: %d)", pl_sd->status.cart[i].amount, item_data->name, item_data->jname, pl_sd->status.cart[i].nameid);
					clif_displaymessage(fd, output);
					*output = '\0';
					counter2 = 0;
					for (j = 0; j < item_data->flag.slot; ++j) {
						if (pl_sd->status.cart[i].card[j]) {
							if ( (item_temp = itemdb_exists(pl_sd->status.cart[i].card[j])) != NULL) {
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
			else {
				snprintf(output, sizeof(output), "%ld item(s) found in %ld kind(s) of items.", (unsigned long)(counter), (unsigned long)(count));
				clif_displaymessage(fd, output);
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return false;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @killer by MouseJstr
/// enable killing players even when not in pvp
///
bool atcommand_killer(int fd, struct map_session_data &sd, const char* command, const char* message)
{

	sd.state.killer = !sd.state.killer;

	if(sd.state.killer)
	  clif_displaymessage(fd, msg_table[241]);
        else
	  clif_displaymessage(fd, msg_table[242]);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @killable by MouseJstr
/// enable other people killing you
///
bool atcommand_killable(int fd, struct map_session_data &sd, const char* command, const char* message)
{

	sd.state.killable = !sd.state.killable;

	if(sd.state.killable)
	  clif_displaymessage(fd, msg_table[242]);
        else
	  clif_displaymessage(fd, msg_table[241]);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @charkillable by MouseJstr
/// enable another player to be killed
///
bool atcommand_charkillable(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;


	if (!message || !*message)
		return false;

	if((pl_sd=map_nick2sd((char *) message)) == NULL)
                return false;

	pl_sd->state.killable = !pl_sd->state.killable;

	if(pl_sd->state.killable)
	  clif_displaymessage(fd, "The player is now killable");
        else
	  clif_displaymessage(fd, "The player is no longer killable");

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// @skillon by MouseJstr
/// turn skills on for the map
///
bool atcommand_skillon(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	maps[sd.block_list::m].flag.noskill = 0;
	clif_displaymessage(fd, msg_table[244]);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @skilloff by MouseJstr
/// Turn skills off on the map
///
bool atcommand_skilloff(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	maps[sd.block_list::m].flag.noskill = 1;
	clif_displaymessage(fd, msg_table[243]);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @npcmove by MouseJstr
/// move a npc
///
bool atcommand_npcmove(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	int x = 0, y = 0;
	struct npc_data *nd = 0;



	if (!message || !*message)
		return false;

	if(sscanf(message, "%d %d %99[^\n]", &x, &y, player_name) < 3) {
		clif_displaymessage(fd, "Usage: @npcmove <X> <Y> <npc_name>");
		return false;
	}

	nd = npc_name2id(player_name);
	if(!nd)
		return false;

	npc_enable(player_name, 0);
	nd->block_list::x = x;
	nd->block_list::y = y;
	npc_enable(player_name, 1);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @addwarp by MouseJstr
/// Create a new static warp point.
///
bool atcommand_addwarp(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128];
	char mapname[128]="";
	char w1[64], w3[64], w4[64];
	int x,y;

	if (!message || !*message)
		return false;

	if(sscanf(message, "%99s %d %d[^\n]", mapname, &x, &y ) < 3)
		return false;

	snprintf(w1,sizeof(w1), "%s,%d,%d", sd.mapname, sd.block_list::x, sd.block_list::y);
	snprintf(w3,sizeof(w3), "%s%d%d%d%d", mapname,sd.block_list::x, sd.block_list::y, x, y);
	snprintf(w4,sizeof(w4), "1,1,%s.gat,%d,%d", mapname, x, y);

	if( npc_parse_warp(w1, "warp", w3, w4) )
	{
		snprintf(output, sizeof(output), "New warp NPC => %s",w3);
		clif_displaymessage(fd, output);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @follow by [MouseJstr]
/// Follow a player .. staying no more then 5 spaces away
///
bool atcommand_follow(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;

	if (!message || !*message)
		return false;

	pl_sd = map_nick2sd(message);
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
/// @dropall by [MouseJstr]
/// Drop all your possession on the ground
///
bool atcommand_dropall(int fd, struct map_session_data &sd, const char* command, const char* message)
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
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @chardropall by [MouseJstr]
/// Throw all the characters possessions on the ground.  Normally
/// done in response to them being disrespectful of a GM
///
bool atcommand_chardropall(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int i;
	struct map_session_data *pl_sd = NULL;


	if (!message || !*message)
		return false;
	if((pl_sd=map_nick2sd((char *) message)) == NULL)
		return false;
	for (i = 0; i < MAX_INVENTORY; ++i) {
		if (pl_sd->status.inventory[i].amount) {
			if(pl_sd->status.inventory[i].equip != 0)
				pc_unequipitem(*pl_sd, i, 3);
			pc_dropitem(*pl_sd,  i, pl_sd->status.inventory[i].amount);
		}
	}

	clif_displaymessage(pl_sd->fd, "Ever play 52 card pickup?");
	clif_displaymessage(fd, "It is done");
	//clif_displaymessage(fd, "It is offical.. your a jerk");

	return true;
}
///////////////////////////////////////////////////////////////////////////////
///
/// @storeall by [MouseJstr]
/// Put everything into storage to simplify your inventory to make debugging easier
///
bool atcommand_storeall(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int i;
	
	if (storage_storageopen(sd) == 1) {
		clif_displaymessage(fd, "run this command again..");
		return true;
	}
	for (i = 0; i < MAX_INVENTORY; ++i) {
		if (sd.status.inventory[i].amount) {
			if(sd.status.inventory[i].equip != 0)
				pc_unequipitem(sd, i, 3);
			storage_storageadd(sd,  i, sd.status.inventory[i].amount);
		}
	}
	storage_storageclose(sd);

	clif_displaymessage(fd, "It is done");
	return true;
}
///////////////////////////////////////////////////////////////////////////////
///
/// @charstoreall by [MouseJstr]
/// A way to screw with players who piss you off
///
bool atcommand_charstoreall(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int i;
	struct map_session_data *pl_sd = NULL;


	if (!message || !*message)
		return false;
	if((pl_sd=map_nick2sd((char *) message)) == NULL)
		return false;

	if (storage_storageopen(*pl_sd) == 1) {
		clif_displaymessage(fd, "Had to open the characters storage window...");
		clif_displaymessage(fd, "run this command again..");
		return true;
	}
	for (i = 0; i < MAX_INVENTORY; ++i) {
		if (pl_sd->status.inventory[i].amount) {
			if(pl_sd->status.inventory[i].equip != 0)
				pc_unequipitem(*pl_sd, i, 3);
			storage_storageadd(*pl_sd,  i, sd.status.inventory[i].amount);
		}
	}
	storage_storageclose(*pl_sd);

	clif_displaymessage(pl_sd->fd, "Everything you own has been put away for safe keeping.");
	clif_displaymessage(pl_sd->fd, "go to the nearest kafka to retrieve it..");
	clif_displaymessage(pl_sd->fd, "   -- the management");

	clif_displaymessage(fd, "It is done");

	return true;
}
///////////////////////////////////////////////////////////////////////////////
///
/// @skillid by [MouseJstr]
/// lookup a skill by name
///
bool atcommand_skillid(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	size_t skillen, idx;

	if (!message || !*message)
		return false;
	skillen = strlen(message);

	for (idx = 0; idx < MAX_SKILL_DB; ++idx)
	{
		if ((skill_db[idx].name && strncasecmp(skill_db[idx].name, message, skillen) == 0) ||
			(skill_db[idx].desc && strncasecmp(skill_db[idx].desc, message, skillen) == 0))
		{
			char output[256];
			snprintf(output, sizeof(output),"skill %ld: %s", (unsigned long)(idx), skill_db[idx].desc);
			clif_displaymessage(fd, output);
		}

	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @useskill by [MouseJstr]
/// A way of using skills without having to find them in the skills menu
///
bool atcommand_useskill(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int skillnum;
	int skilllv;
	int inf;
	char target[255];


	if (!message || !*message)
		return false;
	if(sscanf(message, "%d %d %99[^\n]", &skillnum, &skilllv, target) != 3) {
		clif_displaymessage(fd, "Usage: @useskill <skillnum> <skillv> <target>");
		return false;
	}
	if((pl_sd=map_nick2sd(target)) == NULL) {
		return false;
	}

	inf = skill_get_inf(skillnum);

	if ((inf == 2) || (inf == 1))
		skill_use_pos(&sd, pl_sd->block_list::x, pl_sd->block_list::y, skillnum, skilllv);
	else
		skill_use_id(&sd, pl_sd->block_list::id, skillnum, skilllv);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @skilltree by [MouseJstr]
/// prints the skill tree for a player required to get to a skill
///
bool atcommand_skilltree(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128];
	struct map_session_data *pl_sd = NULL;
	int skillnum, skillidx = -1;
	int meets = 1, j, c=0;
	char target[256];
	struct skill_tree_entry *ent;
	
	if (!message || !*message)
		return false;
	if(sscanf(message, "%d %[^\r\n]", &skillnum, target) != 2)
	{
		clif_displaymessage(fd, "Usage: @skilltree <skillnum> <target>");
		return false;
	}
	if((pl_sd=map_nick2sd(target)) == NULL)
		return false;
	
	c = pc_calc_skilltree_normalize_job(*pl_sd);

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
/// Hand a ring with partners name on it to this char
///
void getring(struct map_session_data &sd)
{
	int flag;
        struct item item_tmp;
	unsigned short item_id = (sd.status.sex==0) ? 2635 : 2634;

        memset(&item_tmp,0,sizeof(item_tmp));
        item_tmp.nameid=item_id;
        item_tmp.identify=1;
	item_tmp.card[0] = 0x00FF;
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
/// @marry by [MouseJstr], fixed by Lupus
/// Marry two players
///
bool atcommand_marry(int fd, struct map_session_data &sd, const char* command, const char* message)
{
  struct map_session_data *pl_sd1 = NULL;
  struct map_session_data *pl_sd2 = NULL;
  char player1[255], player2[255];

	if (!message || !*message || sscanf(message, "%[^,],%[^\r\n]", player1, player2) != 2)
	{
    clif_displaymessage(fd, "Usage: @marry <player1>,<player2>.");
		return false;
  }
	if((pl_sd1=map_nick2sd((char *) player1)) == NULL)
	{
    snprintf(player2, sizeof(player2), "Cannot find player '%s' online", player1);
    clif_displaymessage(fd, player2);
		return false;
  }
	if((pl_sd2=map_nick2sd((char *) player2)) == NULL)
	{
    snprintf(player1, sizeof(player1), "Cannot find player '%s' online", player2);
    clif_displaymessage(fd, player1);
		return false;
  }
	if( pc_marriage(*pl_sd1, *pl_sd2) )
	{
	clif_displaymessage(fd, "They are married.. wish them well");
		clif_wedding_effect(sd);	//wedding effect and music [Lupus]
	// Auto-give named rings (Aru)
	getring(*pl_sd1);
	getring(*pl_sd2);
	return 0;
  }
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @divorce by [MouseJstr], fixed by [Lupus]
/// divorce two players 
///
bool atcommand_divorce(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128];
	char player_name[128]="";
  struct map_session_data *pl_sd = NULL;

	if(!message || !*message || sscanf(message, "%[^\r\n]", player_name) != 1)
	{
    clif_displaymessage(fd, "Usage: @divorce <player>.");
		return false;
  }
	if((pl_sd=map_nick2sd((char *) player_name)) != NULL)
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

#ifdef DMALLOC
uint32 dmark_;
bool atcommand_dmstart(int fd, struct map_session_data &sd, const char* command, const char* message)
{
  dmark_ = dmalloc_mark();

  clif_displaymessage(fd, "debug mark set");

  return true;
}

bool atcommand_dmtick(int fd, struct map_session_data &sd, const char* command, const char* message)
{
  dmalloc_log_changed ( dmark_, 1, 0, 1 ) ;
  dmark_ = dmalloc_mark();
  clif_displaymessage(fd, "malloc changes logged");

  return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///
/// @grind by [MouseJstr]
///
bool atcommand_grind(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int skillnum;
	int inf;
	char target[255];


	if (!message || !*message)
		return false;
	if(sscanf(message, "%s", target) != 1) {
		clif_displaymessage(fd, "Usage: @grind <target>");
		return false;
	}
	if((pl_sd=map_nick2sd(target)) == NULL)
		return false;

	for (skillnum = 1; skillnum < 500; skillnum++) {
		sd.status.sp = sd.status.max_sp;
		atcommand_alive(fd, sd, command, message);

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
/// @grind2 by [MouseJstr]
///
bool atcommand_grind2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int i, x, y, id;


	for (i =  1000; i <2000; ++i) {
		x = sd.block_list::x + (rand() % 10 - 5);
		y = sd.block_list::y + (rand() % 10 - 5);
		id = mob_once_spawn(&sd, "this", x, y, "--ja--", i, 1, "");
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  @changelook by [Celest]
///
bool atcommand_changelook(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int i, j = 0, k = 0;
	int pos[6] = { LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_HEAD_BOTTOM,LOOK_WEAPON,LOOK_SHIELD,LOOK_SHOES };

	if((i = sscanf(message, "%d %d", &j, &k)) < 1) {
		clif_displaymessage(fd, "Usage: @changelook [<position>] <view id> -- [] = optional");
		clif_displaymessage(fd, "Position: 1-Top 2-Middle 3-Bottom 4-Weapon 5-Shield");
		return false;
	} else if (i == 2) {
		if (j < 1) j = 1;
		else if (j > 6) j = 6;	// 6 = Shoes - for beta clients only perhaps
		j = pos[j - 1];
	} else if (i == 1) {	// position not defined, use HEAD_TOP as default
		k = j;	// swap
		j = LOOK_HEAD_TOP;
	}

	clif_changelook(sd,j,k);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Turns on/off AutoLoot for a specific player
///
bool atcommand_autoloot(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	if(sd.state.autoloot) 
	{
		sd.state.autoloot = 0;
		clif_displaymessage(fd, "Autoloot is now off.");
	}
	else 
	{
		sd.state.autoloot = 1;
		clif_displaymessage(fd, "Autoloot is now on.");
	}
	return true;  
}   


///////////////////////////////////////////////////////////////////////////////
///
/// It is made to rain.
///
bool atcommand_rain(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	if (maps[sd.block_list::m].flag.rain)
	{
		maps[sd.block_list::m].flag.rain=0;
		clif_clearweather(sd.block_list::m);
		clif_displaymessage(fd, "The rain has stopped.");
	}
	else
	{
		maps[sd.block_list::m].flag.rain=1;
		clif_weather2(sd.block_list::m,EFFECT_RAIN);
		clif_displaymessage(fd, "It is made to rain.");
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////
///
/// It is made to snow.
///
bool atcommand_snow(int fd, struct map_session_data &sd, const char* command, const char* message)
{

	if (maps[sd.block_list::m].flag.snow)
	{
		maps[sd.block_list::m].flag.snow=0;
		clif_clearweather(sd.block_list::m);
		clif_displaymessage(fd, "Snow has stopped falling.");
	}
	else
	{
		maps[sd.block_list::m].flag.snow=1;
		clif_weather2(sd.block_list::m,EFFECT_SNOW);
		clif_displaymessage(fd, "It is made to snow.");
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Cherry tree snowstorm is made to fall. (Sakura)
///
bool atcommand_sakura(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	if (maps[sd.block_list::m].flag.sakura)
	{
		maps[sd.block_list::m].flag.sakura=0;
		clif_clearweather(sd.block_list::m);
		clif_displaymessage(fd, "Cherry tree leaves are gone.");
	}
	else
	{
		maps[sd.block_list::m].flag.sakura=1;
		clif_weather2(sd.block_list::m,EFFECT_SAKURA);
		clif_displaymessage(fd, "Cherry tree leaves is made to fall.");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Clouds appear.
///
bool atcommand_clouds(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	if (maps[sd.block_list::m].flag.clouds)
	{
		maps[sd.block_list::m].flag.clouds=0;
		clif_clearweather(sd.block_list::m);
		clif_displaymessage(fd, "The clouds has gone.");
	}
	else
	{
		maps[sd.block_list::m].flag.clouds=1;
		clif_weather2(sd.block_list::m,EFFECT_CLOUDS);
		clif_displaymessage(fd, "Clouds appear.");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_clouds2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	if (maps[sd.block_list::m].flag.clouds2)
	{
		maps[sd.block_list::m].flag.clouds2=0;
		clif_clearweather(sd.block_list::m);
		clif_displaymessage(fd, "The clouds has gone.");
	}
	else
	{
		maps[sd.block_list::m].flag.clouds2=1;
		clif_weather2(sd.block_list::m,EFFECT_CLOUDS2);
		clif_displaymessage(fd, "Clouds appear.");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Fog hangs over.
///
bool atcommand_fog(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	if (maps[sd.block_list::m].flag.fog)
	{
		maps[sd.block_list::m].flag.fog=0;
		clif_clearweather(sd.block_list::m);
		clif_displaymessage(fd, "The fog has gone.");
	}
	else
	{
		maps[sd.block_list::m].flag.fog=1;
		clif_weather2(sd.block_list::m, EFFECT_FOG);
		clif_displaymessage(fd, "Fog hangs over.");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Fallen leaves fall.
///
bool atcommand_leaves(int fd, struct map_session_data &sd, const char* command, const char* message)
{

	if (maps[sd.block_list::m].flag.leaves)
	{
		maps[sd.block_list::m].flag.leaves=0;
		clif_clearweather(sd.block_list::m);
		clif_displaymessage(fd, "Leaves no longer fall.");
	}
	else
	{
		maps[sd.block_list::m].flag.leaves=1;
		clif_weather2(sd.block_list::m, EFFECT_LEAVES);
		clif_displaymessage(fd, "Fallen leaves fall.");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Clouds appear.
///
bool atcommand_fireworks(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	if (maps[sd.block_list::m].flag.fireworks) {
		maps[sd.block_list::m].flag.fireworks=0;
		clif_clearweather(sd.block_list::m);
		clif_displaymessage(fd, "Fireworks have burned down.");
	} else {
		maps[sd.block_list::m].flag.fireworks=1;
		clif_weather2(sd.block_list::m, EFFECT_FIRE1);
		clif_weather2(sd.block_list::m, EFFECT_FIRE2);
		clif_weather2(sd.block_list::m, EFFECT_FIRE3);
		clif_displaymessage(fd, "Fireworks are launched.");
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Clearing Weather Effects by Dexity
///
bool atcommand_clearweather(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	maps[sd.block_list::m].flag.rain=0;
	maps[sd.block_list::m].flag.snow=0;
	maps[sd.block_list::m].flag.sakura=0;
	maps[sd.block_list::m].flag.clouds=0;
	maps[sd.block_list::m].flag.clouds2=0;
	maps[sd.block_list::m].flag.fog=0;
	maps[sd.block_list::m].flag.fireworks=0;
	maps[sd.block_list::m].flag.leaves=0;
	clif_clearweather(sd.block_list::m);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Sound Command - plays a sound for everyone! [Codemaster]
///
bool atcommand_sound(int fd, struct map_session_data &sd, const char *command, const char *message)
{
	char sound_file[128]="";

	if(!message || !*message || sscanf(message, "%99[^\n]", sound_file) < 1) {
		clif_displaymessage(fd, "Please, enter a sound filename. (usage: @sound <filename>)");
		return false;
	}

	if(sscanf(message, "%99[^\n]", sound_file) < 1)
		return false;

	if(strstr(sound_file, ".wav") == NULL)
		strcat(sound_file, ".wav");

	clif_soundeffectall(sd, sound_file,0);

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
	virtual int process(struct block_list& bl) const
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

bool atcommand_mobsearch(int fd, struct map_session_data& sd, const char* command, const char* message)
{
	char output[128];
	char mob_name[128]="";
	int mob_id,map_id = 0;



	if (sscanf(message, "%99[^\n]", mob_name) < 0)
		return false;

	if ((mob_id = atoi(mob_name)) == 0)
		 mob_id = mobdb_searchname(mob_name);
	if(mob_id !=-1 && (mob_id <= 1000 || mob_id >= 2000)){
		snprintf(output, sizeof output, "Invalid mob id %s!",mob_name);
		clif_displaymessage(fd, output);
		return true;
	}
	if(mob_id == atoi(mob_name) && mob_db[mob_id].jname)
				strcpy(mob_name,mob_db[mob_id].jname);	// --ja--
//				strcpy(mob_name,mob_db[mob_id].name);	// --en--

	map_id = sd.block_list::m;

	snprintf(output, sizeof output, "Mob Search... %s %s",
		mob_name, sd.mapname);
	clif_displaymessage(fd, output);

	CMap::foreachinarea( CAtMobSearch(mob_id, fd),
		map_id, 0, 0, maps[map_id].xs-1, maps[map_id].ys-1, BL_MOB);
//	map_foreachinarea(atmobsearch_sub, 
//		map_id, 0, 0, maps[map_id].xs-1, maps[map_id].ys-1, BL_MOB, 
//		mob_id, fd);
//	va_list ap=NULL;
//	atmobsearch_sub(sd,ap);		// î‘çÜÉäÉZÉbÉg

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
	virtual int process(struct block_list& bl) const
	{
		map_clearflooritem(bl.id);
		return 0;
	}
};

bool atcommand_cleanmap(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	CMap::foreachinarea( CAtCleanMap(),
		sd.block_list::m, ((int)sd.block_list::x)-AREA_SIZE*2, ((int)sd.block_list::y)-AREA_SIZE*2, ((int)sd.block_list::x)+AREA_SIZE*2, ((int)sd.block_list::y)+AREA_SIZE*2, BL_ITEM);
//	map_foreachinarea(atcommand_cleanmap_sub, 
//		sd.block_list::m, ((int)sd.block_list::x)-AREA_SIZE*2, ((int)sd.block_list::y)-AREA_SIZE*2, ((int)sd.block_list::x)+AREA_SIZE*2, ((int)sd.block_list::y)+AREA_SIZE*2, BL_ITEM);
	clif_displaymessage(fd, "All dropped items have been cleaned up.");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// NPC/PETÇ…òbÇ≥ÇπÇÈ
///
bool atcommand_npctalk(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char name[128],mes[128],output[256];
	struct npc_data *nd;

//	if(sscanf(message, "%s %99[^\n]", name, mes) < 2)
//		return false;
	if( (sscanf(message, "\"%[^\"]\" %99[^\n]", name, mes) < 2) && //orn
		(sscanf(message, "%99s %99[^\n]", name, mes) < 2))
		return false;

	if (!(nd = npc_name2id(name)))
		return false;
	
//	clif_message(&nd->bl, mes);
	sscanf(name, "%[^#]#[^\n]", name);
	snprintf(output, sizeof(output), "%s: %s", name, mes);
	clif_message(*nd, output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_pettalk(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char mes[128],temp[128];

	if(!sd.status.pet_id || !sd.pd)
		return false;

	if (sscanf(message, "%99[^\n]", mes) < 1)
		return false;

	snprintf(temp, sizeof(temp), "%s : %s", sd.pd->pet.name, mes);
	clif_message(*sd.pd, temp);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @users
/// ÉTÅ[ÉoÅ[ì‡ÇÃêlêîÉ}ÉbÉvÇï\é¶Ç≥ÇπÇÈ
/// éËî≤Ç´ÇÃÇΩÇﬂâòÇ≠Ç»Ç¡ÇƒÇ¢ÇÈÇÃÇÕédólÇ≈Ç∑ÅB
///
static struct dbt *users_db;
static int users_all;
class CClifAtUsers : public CClifProcessor
{
public:
	CClifAtUsers()	{}
	virtual ~CClifAtUsers()	{}
	virtual bool process(struct map_session_data& sd) const
	{
		size_t users = (size_t)strdb_search(users_db,sd.mapname) + 1;
		users_all++;
		strdb_insert(users_db,sd.mapname,(void *)users);
		return 0;
	}
};
class CDBAtUsers : public CDBProcessor
{
	struct map_session_data &sd;
public:
	CDBAtUsers(struct map_session_data &s) : sd(s)	{}
	virtual ~CDBAtUsers()	{}
	virtual bool process(void *key, void *data) const
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "%s : %ld (%ld%%)",(char *)key,(unsigned long)((ssize_t)((size_t)data)),(unsigned long)((ssize_t)((size_t)data) * 100 / users_all));
		clif_displaymessage(sd.fd, buf);
		return true;
	}
};
int atcommand_users_sub2(db_iterator iter, struct map_session_data& sd)
{
	char buf[256];
	for( ;iter; ++iter)
	{
		snprintf(buf,sizeof(buf), "%s : %ld (%ld%%)",(char *)iter.key(),(unsigned long)((ssize_t)((size_t)iter.data())),(unsigned long)((ssize_t)((size_t)iter.data())* 100 / users_all));
		clif_displaymessage(sd.fd,buf);
	}
	return 0;
}
bool atcommand_users(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char buf[256];
	users_all = 0;
	users_db = strdb_init(24);

	clif_foreachclient( CClifAtUsers() );
//	clif_foreachclient(atcommand_users_sub1);
	
	strdb_foreach(users_db, CDBAtUsers(sd) );
//	atcommand_users_sub2( users_db, sd);
//	strdb_foreach(users_db, atcommand_users_sub2, &sd);

	snprintf(buf,sizeof(buf), "all : %d",users_all);
	clif_displaymessage(fd,buf);
	strdb_final(users_db,NULL);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_summon(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char name[128];
	uint32 mob_id = 0;
	int x = 0;
	int y = 0;
	uint32 id = 0;
	int duration = 0;
	struct mob_data *md;
	unsigned long tick=gettick();

	if(!message || !*message)
		return false;
	if(sscanf(message, "%99s %u", name, &duration) < 1)
		return false;

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

	if((md=(struct mob_data *)map_id2bl(id)))
	{
		md->master_id=sd.block_list::id;
		md->state.special_mob_ai=1;
		md->mode = mob_db[md->class_].mode|0x04;
		md->deletetimer=add_timer(tick+duration*60000,mob_timer_delete,id,0);
		clif_misceffect2(*md,344);
	}
	clif_skill_poseffect(sd,AM_CALLHOMUN,1,x,y,tick);

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// @adjcmdlvl by [MouseJstr]
/// Temp adjust the GM level required to use a GM command
/// Used during beta testing to allow players to use GM commands
/// for short periods of time
///
bool atcommand_adjcmdlvl(int fd, struct map_session_data &sd, const char* command, const char* message)
{
    int i, newlev;
    char cmd[128];


    if (!message || !*message || sscanf(message, "%d %s", &newlev, cmd) != 2) {
        clif_displaymessage(fd, "Usage: @adjcmdlvl <lvl> <command>.");
        return false;
    }

    for (i = 0; (atcommand_info[i].command) && atcommand_info[i].type != AtCommand_None; ++i)
        if(strcasecmp(cmd, atcommand_info[i].command+1) == 0) {
            atcommand_info[i].level = newlev;
            clif_displaymessage(fd, "@command level changed.");
            return true;
        }

    clif_displaymessage(fd, "@command not found.");
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @adjgmlvl by [MouseJstr]
/// Change GM level
///
bool atcommand_adjgmlvl(int fd, struct map_session_data &sd, const char* command, const char* message)
{
    int newlev;
    char user[128];
	struct map_session_data *pl_sd;

	*user = 0;
    if (!message || !*message || sscanf(message, "%d %[^\r\n]", &newlev, user) != 2) {
        clif_displaymessage(fd, "Usage: @adjgmlvl <lvl> <user>.");
    }
	else if( sd.status.gm_level <= newlev )
	{
		clif_displaymessage(fd, "You cannot grant higher or equal gm_level than your own.");
	}
	else if( (pl_sd=map_nick2sd(user)) == NULL )
	{
		clif_displaymessage(fd, "Player not found.");
	}
	else if( sd.status.gm_level <= pl_sd->status.gm_level )
	{
		clif_displaymessage(fd, "You are not allowed to modify the gm_level of higher or equal priorized players.");
	}
	else
	{
		pl_sd->status.gm_level = newlev;
		clif_displaymessage(pl_sd->fd, "Your gm_level has been changed.");
		clif_displaymessage(fd, "gm_level changed.");
		return true;
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////
///
/// @trade by [MouseJstr]
/// Open a trade window with a remote player
/// If I have to jump to a remote player one more time, I am gonna scream!
///
bool atcommand_trade(int fd, struct map_session_data &sd, const char* command, const char* message)
{
    struct map_session_data *pl_sd = NULL;


    if (!message || !*message)
        return false;
    if((pl_sd=map_nick2sd((char *) message)) != NULL) {
        trade_traderequest(sd, pl_sd->block_list::id);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @setbattleflag by [MouseJstr]
/// set a config flag without having to reboot
///
bool atcommand_setbattleflag(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char flag[128];
	char value[128];

	if (!message || !*message || sscanf(message, "%s %s", flag, value) != 2) {
        	clif_displaymessage(fd, "Usage: @setbattleflag <flag> <value>.");
        	return false;
    	}

	if( config_set_value(flag, value) )
	{
		clif_displaymessage(fd, "config set as requested");
		config_validate();
	}
	else
		clif_displaymessage(fd, "unknown config flag");
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// @unmute [Valaris]
///
bool atcommand_unmute(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;

	
	if(!config.muting_players) {
		clif_displaymessage(fd, "Please enable the muting system before using it.");
		return true;
	}

	if (!message || !*message)
        	return false;

	if((pl_sd=map_nick2sd((char *) message)) != NULL) {
		if(pl_sd->sc_data[SC_NOCHAT].timer!=-1) {
			pl_sd->status.manner = 0; // have to set to 0 first [celest]
			status_change_end(pl_sd,SC_NOCHAT,-1);
			clif_displaymessage(sd.fd,"Player unmuted");
		}
		else
			clif_displaymessage(sd.fd,"Player is not muted");
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @uptime by MC Cameri
///
bool atcommand_uptime(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128];
	unsigned long days, hours, minutes,seconds;


	uptime::getvalues(days, hours, minutes,seconds);

	snprintf(output, sizeof(output), msg_table[245], days, hours, minutes, seconds);
	clif_displaymessage(fd,output);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @changesex <sex>
/// => Changes one's sex. Argument sex can be
/// 0 or 1, m or f, male or female.
///
bool atcommand_changesex(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	chrif_char_ask_name(sd.status.account_id,sd.status.name, 5,0,0,0,0,0,0);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @mute - Mutes a player for a set amount of time
///
bool atcommand_mute(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	if(!config.muting_players) {
		clif_displaymessage(fd, "Please enable the muting system before using it.");
		return true;
	}

	struct map_session_data *pl_sd = NULL;
	int manner;
	
	if(!message || !*message || sscanf(message, "%d %99[^\n]", &manner, player_name) < 1) {
		clif_displaymessage(fd, "Usage: @mute <time> <character name>.");
		return false;
	}


	if( (pl_sd = map_nick2sd(player_name)) == NULL)
	{
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return false;
	}
	else if (pl_sd->isGM() > sd.isGM())
	{
		clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
		return false;
	}
	else
	{
		clif_GM_silence(sd, *pl_sd, 0);
		pl_sd->status.manner -= manner;
		if(pl_sd->status.manner < 0)
			status_change_start(pl_sd,SC_NOCHAT,0,0,0,0,0,0);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @refresh (like @jumpto <<yourself>>)
///
bool atcommand_refresh(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	
	//pc_setpos(sd, sd.mapname, sd.block_list::x, sd.block_list::y, 3);
	clif_refresh(sd);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @petid <part of pet name>
/// => Displays a list of matching pets.
///
bool atcommand_petid(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char searchtext[128];
	char temp0[128];
	char temp1[128];
	int cnt = 0, i = 0;



	if (!message || !*message)
		return false;
	if (sscanf(message, "%99s", searchtext) < 1)
		return false;
	basics::tolower(searchtext);
	snprintf(temp0, sizeof(temp0), "Search results for: %s", searchtext);
	clif_displaymessage(fd,temp0);
	while (i < MAX_PET_DB) {
		strcpytolower(temp1,pet_db[i].name);
		strcpytolower(temp0,pet_db[i].jname);

		if (strstr(temp1, searchtext) || strstr(temp0, searchtext) ) {
  			snprintf(temp0, sizeof(temp0), "ID: %i -- Name: %s", pet_db[i].class_,
     			pet_db[i].jname);
  			if (cnt >= 100) { // Only if there are custom pets
	  			clif_displaymessage(fd, "Be more specific, can't send more than"
					" 100 results.");
			} else {
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
/// @identify
/// => GM's magnifier.
///
bool atcommand_identify(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int i,num;



	for(i=num=0; i<MAX_INVENTORY; ++i){
		if(sd.status.inventory[i].nameid > 0 && sd.status.inventory[i].identify!=1){
			num++;
		}
	}
	if (num > 0) {
		clif_item_identify_list(sd);
	} else {
		clif_displaymessage(fd,"There are no items to appraise.");
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @gmotd (Global MOTD)
/// by davidsiaw :P
///
bool atcommand_gmotd(int fd, struct map_session_data &sd, const char* command, const char* message)
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
/// 
bool atcommand_misceffect(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int effect = 0;
	
	if (!message || !*message)
		return false;
	if (sscanf(message, "%d", &effect) < 1)
		return false;
	clif_misceffect(sd,effect);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
int charid2sessionid(uint32 charid)
{
	size_t session_id;
   struct map_session_data *pl_sd = NULL;
	for(session_id=0;session_id<fd_max;session_id++)
	{
		if(session[session_id] && (pl_sd=(struct map_session_data *)session[session_id]->user_session) && pl_sd->state.auth)
		{
			if(pl_sd->status.char_id==charid)
				break;
      }
   }
   return session_id;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
int accountid2sessionid(uint32 accountid)
{
	size_t session_id;
   struct map_session_data *pl_sd = NULL;

	for(session_id=0;session_id<fd_max;session_id++)
	{
		if(session[session_id] && (pl_sd=(struct map_session_data *)session[session_id]->user_session) && pl_sd->state.auth)
		{
			if(pl_sd->status.account_id==accountid)
				break;
      }
   }
   return session_id;
}


///////////////////////////////////////////////////////////////////////////////
///
/// Jump to a player by PID number
/// Original by Dino9021
///  Added in by nsstrunks
///
bool atcommand_jumptoid(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	char output[128];
   int cid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

	if(!message || (cid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player CID (usage: @jumptoid/@warptoid/@gotoid <char id>).");
      return false;
   }
   cid=atoi(message);

	if((session_id=charid2sessionid(cid))!=0){
      if ((pl_sd = (struct map_session_data *) session[session_id]->user_session) != NULL) {
         if (pl_sd->block_list::m < map_num && maps[pl_sd->block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
            clif_displaymessage(fd, msg_table[247]);
            return false;
         }
         if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
            clif_displaymessage(fd, msg_table[248]);
            return false;
         }
         pc_setpos(sd, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y, 3);
         snprintf(output, sizeof(output), msg_table[4], pl_sd->status.name); // Jump to %s
         clif_displaymessage(fd, output);
      } else {
         clif_displaymessage(fd, msg_table[154]); // Character not found.
         return false;
      }
	} else {
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, cid = %d\n",session_id,cid);
   return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// Jump to a player by PID number
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_jumptoid2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char output[128];
	char player_name[128]="";
   int aid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

	if(!message || (aid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player AID (usage: @jumptoid/@warptoid/@gotoid <account id>).");
      return false;
   }
   aid=atoi(message);

	if((session_id=accountid2sessionid(aid))!=0) {
      if ((pl_sd = (struct map_session_data *) session[session_id]->user_session) != NULL) {
         if (pl_sd->block_list::m < map_num && maps[pl_sd->block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
            clif_displaymessage(fd, msg_table[247]);
            return false;
         }
         if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
            clif_displaymessage(fd, msg_table[248]);
            return false;
         }
         pc_setpos(sd, pl_sd->mapname, pl_sd->block_list::x, pl_sd->block_list::y, 3);
         snprintf(output, sizeof(output), msg_table[4], pl_sd->status.name); // Jump to %s
         clif_displaymessage(fd, output);
      } else {
         clif_displaymessage(fd, msg_table[154]); // Character not found.
         return false;
      }
	}else{
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, aid = %d\n",session_id,aid);
   return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Recall a player by PID number
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_recallid(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	char output[128];
   int cid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

	if(!message || (cid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player CID (usage: @recallid <char id>).");
      return false;
   }
   cid=atoi(message);

	if((session_id=charid2sessionid(cid))!=0){
      if ((pl_sd = (struct map_session_data *) session[session_id]->user_session) != NULL) {
         if (sd.isGM() >= pl_sd->isGM()) { // you can recall only lower or same level
            if (pl_sd->block_list::m < map_num && maps[pl_sd->block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
               clif_displaymessage(fd, msg_table[247]);
               return false;
            }
            if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
               clif_displaymessage(fd, msg_table[248]);
               return false;
            }
            pc_setpos(*pl_sd, sd.mapname, sd.block_list::x, sd.block_list::y, 2);
            snprintf(output, sizeof(output), msg_table[46], pl_sd->status.name); // Jump to %s
            clif_displaymessage(fd, output);
         } else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return false;
         }
      } else {
         clif_displaymessage(fd, msg_table[154]); // Character not found.
         return false;
      }
	}else{
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, cid = %d\n",session_id,cid);
   return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Recall a player by PID number
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_recallid2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
	char output[128];
   int aid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

	if(!message || (aid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player AID (usage: @recallid2 <account id>).");
      return false;
   }
   aid=atoi(message);

	if((session_id=accountid2sessionid(aid))!=0) {
      if ((pl_sd = (struct map_session_data *) session[session_id]->user_session) != NULL) {
         if (sd.isGM() >= pl_sd->isGM()) { // you can recall only lower or same level
            if (pl_sd->block_list::m < map_num && maps[pl_sd->block_list::m].flag.nowarpto && config.any_warp_GM_min_level > sd.isGM()) {
				clif_displaymessage(fd, msg_table[247]);
				return false;
            }
            if (sd.block_list::m < map_num && maps[sd.block_list::m].flag.nowarp && config.any_warp_GM_min_level > sd.isGM()) {
               clif_displaymessage(fd, msg_table[248]);
               return false;
            }
            pc_setpos(*pl_sd, sd.mapname, sd.block_list::x, sd.block_list::y, 2);
            snprintf(output, sizeof(output), msg_table[46], pl_sd->status.name); // Jump to %s
            clif_displaymessage(fd, output);
         } else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return false;
         }
      } else {
         clif_displaymessage(fd, msg_table[154]); // Character not found.
         return false;
      }
	}else{
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, aid = %d\n",session_id,aid);
   return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Kick a player by PID number
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_kickid(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
   struct map_session_data *pl_sd;
   int cid=0;
   int session_id=0;
	if(!message || (cid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player CID (usage: @kickid <char id>).");
      return false;
   }
   cid=atoi(message);

	if((session_id=charid2sessionid(cid))!=0){
      if ((pl_sd = (struct map_session_data *) session[session_id]->user_session) != NULL) {
			if(sd.isGM() >= pl_sd->isGM()){ // you can kick only lower or same gm level
            clif_GM_kick(sd, *pl_sd, 1);
			} else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return false;
         }
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return false;
      }
	} else {
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, cid = %d\n",session_id,cid);
   return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Kick a player by PID number
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_kickid2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
   struct map_session_data *pl_sd;
   int aid=0;
   int session_id=0;
	if(!message || (aid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player AID (usage: @kickid2 <account id>).");
      return false;
   }
   aid=atoi(message);

	if((session_id=accountid2sessionid(aid))!=0) {
      if ((pl_sd = (struct map_session_data *) session[session_id]->user_session) != NULL) {
			if(sd.isGM() >= pl_sd->isGM()){ // you can kick only lower or same gm level
            clif_GM_kick(sd, *pl_sd, 1);
			} else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return false;
         }
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return false;
      }
	} else {
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, aid = %d\n",session_id,aid);
   return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Revive a player by PID number
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_reviveid(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
   int cid=0;
   int session_id=0;
   struct map_session_data *pl_sd;
	if(!message || (cid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player CID (usage: @reviveid <char id>).");
      return false;
   }
   cid=atoi(message);

	if((session_id=charid2sessionid(cid))!=0) {
      if ((pl_sd = (struct map_session_data *) session[session_id]->user_session) != NULL) {
         pl_sd->status.hp = pl_sd->status.max_hp;
         pc_setstand(*pl_sd);
         if (config.pc_invincible_time > 0)
            pc_setinvincibletimer(sd, config.pc_invincible_time);
         clif_updatestatus(*pl_sd, SP_HP);
         clif_updatestatus(*pl_sd, SP_SP);
         clif_resurrection(*pl_sd, 1);
         clif_displaymessage(fd, msg_table[51]); // Character revived.
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return false;
      }
	}else{
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, cid = %d\n",session_id,cid);
   return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Revive a player by PID number
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_reviveid2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
   int aid=0;
   int session_id=0;
   struct map_session_data *pl_sd;
	if(!message || (aid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player AID (usage: @reviveid2 <account id>).");
      return false;
   }
   aid=atoi(message);
   if ((session_id=accountid2sessionid(aid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->user_session) != NULL) {
         pl_sd->status.hp = pl_sd->status.max_hp;
         pc_setstand(*pl_sd);
         if (config.pc_invincible_time > 0)
            pc_setinvincibletimer(sd, config.pc_invincible_time);
         clif_updatestatus(*pl_sd, SP_HP);
         clif_updatestatus(*pl_sd, SP_SP);
         clif_resurrection(*pl_sd, 1);
         clif_displaymessage(fd, msg_table[51]); // Character revived.
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return false;
      }
	} else {
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, aid = %d\n",session_id,aid);
   return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Kill a player by PID number
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_killid(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
   int cid=0;
   int session_id=0;
   struct map_session_data *pl_sd;
	if(!message || (cid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player CID (usage: @killid <char id>).");
      return false;
   }
   cid=atoi(message);
	if((session_id=charid2sessionid(cid))!=0){
      if ((pl_sd = (struct map_session_data *) session[session_id]->user_session) != NULL) {
         if (sd.isGM() >= pl_sd->isGM()) { // you can kill only lower or same level
            pc_damage(*pl_sd, pl_sd->status.hp + 1,NULL);
            clif_displaymessage(fd, msg_table[14]); // Character killed.
         } else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return false;
         }
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return false;
      }
	} else {
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, cid = %d\n",session_id,cid);
   return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Kill a player by PID number
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_killid2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char player_name[128]="";
   int aid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

	if(!message || (aid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player AID (usage: @killid2 <account id>).");
      return false;
   }
   aid=atoi(message);

	if((session_id=accountid2sessionid(aid))!=0){
      if ((pl_sd = (struct map_session_data *) session[session_id]->user_session) != NULL) {
         if (sd.isGM() >= pl_sd->isGM()) { // you can kill only lower or same level
            pc_damage(*pl_sd, pl_sd->status.hp + 1,NULL);
            clif_displaymessage(fd, msg_table[14]); // Character killed.
         } else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return false;
         }
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return false;
      }
	} else {
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, aid = %d\n",session_id,aid);
   return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///  Make a player killable, by PID
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_charkillableid(int fd, struct map_session_data& sd,const char* command, const char* message)
{
   struct map_session_data *pl_sd = NULL;
	uint32 cid=0;
	uint32 session_id=0;

   if (!message || (cid = atoi(message)) == 0  || !*message)
		return false;

   cid=atoi(message);
   if ((session_id=charid2sessionid(cid))!=0)
   {
      if((pl_sd= (struct map_session_data *) session[session_id]->user_session) == NULL)
			return false;

		pl_sd->state.killable = !pl_sd->state.killable;
		if(pl_sd->state.killable)
        clif_displaymessage(fd, "The player is now killable");
           else
        clif_displaymessage(fd, "The player is no longer killable");
   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, cid = %d\n",session_id,cid);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///  Make a player killable, by PID
/// Original by Dino9021
/// Added in by nsstrunks
///
bool atcommand_charkillableid2(int fd, struct map_session_data &sd, const char* command, const char* message)
{
   struct map_session_data *pl_sd = NULL;
   int aid=0;
   int session_id=0;

   if (!message || (aid = atoi(message)) == 0 || !*message)
      return false;

   aid=atoi(message);

	if((session_id=accountid2sessionid(aid))!=0){
      if((pl_sd= (struct map_session_data *) session[session_id]->user_session) == NULL)
                   return false;

      pl_sd->state.killable = !pl_sd->state.killable;

      if(pl_sd->state.killable)
        clif_displaymessage(fd, "The player is now killable");
           else
        clif_displaymessage(fd, "The player is no longer killable");
   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
	//ShowMessage("Session_id = %d, aid = %d\n",session_id,aid);
   return true;
}


///////////////////////////////////////////////////////////////////////////////
///
///  Mail System commands by [Valaris]
///
bool atcommand_checkmail(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	return chrif_mail_check(sd, true);
}
bool atcommand_listmail(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	return chrif_mail_fetch(sd, true);
}
bool atcommand_listnewmail(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	return chrif_mail_fetch(sd, false);
}
bool atcommand_readmail(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int index;
	if (!message || !*message) {
		clif_displaymessage(sd.fd,"You must specify a message number.");
		return true;
	}
	index = atoi(message);
	if (index < 1) {
		clif_displaymessage(sd.fd,"Message number cannot be negative or zero.");
		return 0;
	}
	return chrif_mail_read(sd, index);
}
bool atcommand_deletemail(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int index;
	if (!message || !*message) {
		clif_displaymessage(sd.fd,"You must specify a message number.");
		return true;
	}
	index = atoi(message);
	if (index < 1) {
		clif_displaymessage(sd.fd,"Message number cannot be negative or zero.");
		return 0;
	}
	return chrif_mail_delete(sd, index);
}
bool atcommand_sendmail(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char name[32],text[128];

	if (!message || !*message) {
		clif_displaymessage(sd.fd,"You must specify a recipient and a message.");
		return true;
	}

	if ((sscanf(message, "\"%[^\"]\" %79[^\n]", name, text) < 2) &&
		(sscanf(message, "%23s %79[^\n]", name, text) < 2)) {
		clif_displaymessage(sd.fd,"You must specify a recipient and a message.");
		return true;
	}
	return chrif_mail_send(sd, name, "", text);
}

///////////////////////////////////////////////////////////////////////////////
///
///  Refresh online command for SQL [Valaris]
/// Will refresh and check online column of
/// players and set correctly.
///
bool atcommand_refreshonline(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char_online_check();
	return true;
}



///////////////////////////////////////////////////////////////////////////////
///
///  Show Monster DB Info   v 1.0
/// originally by [Lupus] eAthena
///
bool atcommand_mobinfo(int fd, struct map_session_data &sd, const char* command, const char* message)
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

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: @mobinfo <monster_name_or_monster_ID>).");
		return false;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(message)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(message));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_table[40]); // Invalid monster ID or name.
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
///  Show Items DB Info   v 1.0
/// originally by [Lupus] eAthena
///
bool atcommand_iteminfo(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char *itype[12] = {"Potion/Food", "BUG!", "Usable", "Etc", "Weapon", "Protection", "Card", "Egg", "Pet Acessory", "BUG!", "Arrow", "Lure/Scroll"};
	char output[128];

	struct item_data *item_data;
	int item_id=0;

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter Item name or its ID (usage: @iteminfo <item_name_or_ID>).");
		return false;
	}

	if ((item_data = itemdb_searchname(message)) != NULL ||
	    (item_data = itemdb_exists(atoi(message))) != NULL)
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
///  @adopt by [Veider]
/// adopt a novice
///
bool atcommand_adopt(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd1 = NULL;
	struct map_session_data *pl_sd2 = NULL;
	struct map_session_data *pl_sd3 = NULL;
	char player1[128], player2[128], player3[128];
	
	if (!message || !*message)
		return false;
	if (sscanf(message, "%[^,],%[^,],%[^\r\n]", player1, player2, player3) != 3)
	{
		clif_displaymessage(fd, "usage: @adopt <player1>,<player2>,<player3>.");
		return false;
	}
	ShowMessage("Adopting: --%s--%s--%s--\n",player1,player2,player3);
	
	if((pl_sd1=map_nick2sd((char *) player1)) == NULL)
	{
		snprintf(player2, sizeof(player2), "Cannot find player %s online", player1);
		clif_displaymessage(fd, player2);
		return false;
	}
	if((pl_sd2=map_nick2sd((char *) player2)) == NULL)
	{
		snprintf(player1, sizeof(player1), "Cannot find player %s online", player2);
		clif_displaymessage(fd, player1);
		return false;
	}
	if((pl_sd3=map_nick2sd((char *) player3)) == NULL)
	{
		snprintf(player1, sizeof(player1), "Cannot find player %s online", player3);
		clif_displaymessage(fd, player1);
		return false;
	}
	if((pl_sd1->status.base_level < 70) || (pl_sd2->status.base_level < 70))
	{
		clif_displaymessage(fd, "They are too young to be parents!");
		return false;
	}
	if( pc_adoption(*pl_sd1, *pl_sd2, *pl_sd3) )
	{
		clif_displaymessage(fd, "They are family.. wish them luck");
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_version(int fd, struct map_session_data &sd, const char* command, const char* message)
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
/// @mutearea by MouseJstr
///
class CAtMuteArea : public CMapProcessor
{
	uint32 id;
	int time;
public:
	CAtMuteArea(uint32 i, int t) : id(i), time(t)	{}
	~CAtMuteArea()	{}
	virtual int process(struct block_list& bl) const
	{
		if(bl.type==BL_PC)
		{
			struct map_session_data &sd = (struct map_session_data &)bl;
			if( id != bl.id && !sd.isGM() )
			{
				sd.status.manner -= time;
				if(sd.status.manner < 0)
					status_change_start(&sd,SC_NOCHAT,0,0,0,0,0,0);
			}
			return 0;
		}
		return 1;
	}
};
bool atcommand_mutearea(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int time;

	if(!config.muting_players) {
		clif_displaymessage(fd, "Please enable the muting system before using it.");
		return true;
	}

	time = atoi(message);
	if (time <= 0)
		time = 15; // 15 minutes default

	CMap::foreachinarea( CAtMuteArea(sd.block_list::id, time),
		sd.block_list::m,  ((int)sd.block_list::x)-AREA_SIZE, ((int)sd.block_list::y)-AREA_SIZE,  ((int)sd.block_list::x)+AREA_SIZE, ((int)sd.block_list::y)+AREA_SIZE, BL_PC);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @shuffle by MouseJstr
///
class CAtShuffle : public CMapProcessor
{
public:
	CAtShuffle()	{}
	~CAtShuffle()	{}
	virtual int process(struct block_list& bl) const
	{
		struct map_session_data &sd = (struct map_session_data &) bl;
		if( bl.type==BL_PC && !sd.isGM())
			pc_setpos(sd, sd.mapname, rand() % 399 + 1, rand() % 399 + 1, 3);
		return 0;
	}
};
bool atcommand_shuffle(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	if (strcmp(message, "area")== 0)
	{
		CMap::foreachinarea( CAtShuffle(),
			sd.block_list::m, ((int)sd.block_list::x)-AREA_SIZE, ((int)sd.block_list::y)-AREA_SIZE, ((int)sd.block_list::x)+AREA_SIZE, ((int)sd.block_list::y)+AREA_SIZE, BL_PC);
	}
	else if (strcmp(message, "map")== 0)
	{
		CMap::foreachinarea( CAtShuffle(),
			sd.block_list::m, 0, 0, maps[sd.block_list::m].xs, maps[sd.block_list::m].ys, BL_PC);
	}
	else if (strcmp(message, "world") == 0)
	{
		struct map_session_data *pl_sd;
		size_t i;
		CAtShuffle cs;
		for (i = 0; i < fd_max; ++i) 
			if(session[i] && (pl_sd = (struct map_session_data *)session[i]->user_session) != NULL && pl_sd->state.auth)
			{
				cs.process(*pl_sd);
			}
	}
	else
		clif_displaymessage(fd, "options are area, map, or world");
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// 
bool atcommand_rates(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char buf[256];
	snprintf(buf, sizeof(buf), "Experience rates: Base %lf.1x / Job %lf.1x",
		(double)config.base_exp_rate/100., (double)config.job_exp_rate/100.);
	clif_displaymessage(fd, buf);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///
/// @me by lordalfa
/// => Displays the OUTPUT string on top of 
///    the Visible players Heads.
///
bool atcommand_me(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char tempmes[256];
	char output[256];
	if (!message || !*message)
	{
		clif_displaymessage(fd, "Please, enter a message (usage: @me <message>).");
		return false;
	}
	sscanf(message, "%199[^\n]", tempmes);
	snprintf(output, 256, "** %s %s **", sd.status.name, tempmes);
    clif_disp_overhead(sd, output);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @size
///
bool atcommand_size(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	int size=0;

	if (!message || !*message)
		return false;

	if (sscanf(message,"%d", &size) < 1)
		return false;

	if(size==0 || sd.state.viewsize) {
		sd.state.viewsize=0;
		pc_setpos(sd, sd.mapname, sd.block_list::x, sd.block_list::y, 3);
	}
	else if(size==1) {
		sd.state.viewsize=1;
		clif_specialeffect(sd,420,0);
	} else if(size==2) {
		sd.state.viewsize=2;
		clif_specialeffect(sd,422,0);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @fakename
/// => Gives your character a fake name.
///
bool atcommand_fakename(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	char name[24];
	if((!message || !*message) && strlen(sd.fakename) > 1) {
		sd.fakename[0]='\0';
		pc_setpos(sd, sd.mapname, sd.block_list::x, sd.block_list::y, 3);
		clif_displaymessage(sd.fd,"Returned to real name.");
		return true;
	}

	if (!message || !*message) {
		clif_displaymessage(sd.fd,"You must enter a name.");
		return true;
	}


	if (sscanf(message, "%23[^\n]", name) < 1) {
		return true;
	}
	
	if(strlen(name) < 2) {
		clif_displaymessage(sd.fd,"Fake name must be at least two characters.");
		return true;
	}
	
	strcpy(sd.fakename,name);
	clif_charnameack(-1, sd, true);
	clif_displaymessage(sd.fd,"Fake name enabled.");
	
	return true;

}

///////////////////////////////////////////////////////////////////////////////
///
/// @mapflag [flagap name] [1|0|on|off] [map name] by Lupus
/// => Shows information about the map flags [map name]
/// Also set flags
///
bool atcommand_mapflag(int fd, struct map_session_data &sd, const char* command, const char* message)
{
// WIP
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///
/// @monsterignore
/// => Makes monsters ignore you. [Valaris]
///
bool atcommand_monsterignore(int fd, struct map_session_data &sd, const char* command, const char* message)
{
	if(!sd.state.monster_ignore) {
		sd.state.monster_ignore=1;
		clif_displaymessage(sd.fd, "Monsters will now ignore you.");
	} else {
		sd.state.monster_ignore=0;
		clif_displaymessage(sd.fd, "Monsters are no longer ignoring you.");
	}
	return true;
}
