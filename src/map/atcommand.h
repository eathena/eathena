
#ifndef _ATCOMMAND_H_
#define _ATCOMMAND_H_

#include "utils.h"


enum AtCommandType
{
	AtCommand_AddWarp = 0,
	AtCommand_AdjCmdLvl,
	AtCommand_AdjGmLvl,
	AtCommand_Adopt,
	AtCommand_AgitEnd,
	AtCommand_AgitStart,
	AtCommand_Alive,
	AtCommand_AllSkill,
	AtCommand_AutoLoot,
	AtCommand_BaseLevelUp,
	AtCommand_Broadcast,
	AtCommand_CartList,
	AtCommand_ChangeLook,
	AtCommand_ChangeSex,
	AtCommand_CharBan,
	AtCommand_CharBlock,
	AtCommand_CharReset,
	AtCommand_CharUnBan,
	AtCommand_CharUnBlock,
	AtCommand_CheckMail,
	AtCommand_CleanMap,
	AtCommand_ClearWeather,
	AtCommand_Clouds,
	AtCommand_Clouds2,
	AtCommand_Day,
	AtCommand_DeleteMail,
	AtCommand_DeleteItem,
	AtCommand_Die,
	AtCommand_Disguise,
	AtCommand_DisguiseAll,
	AtCommand_Divorce,
	AtCommand_DMStart,
	AtCommand_DMTick,
	AtCommand_Doom,
	AtCommand_DoomMap,
	AtCommand_Dropall,
	AtCommand_Dye,
	AtCommand_Effect,
	AtCommand_EMail,
	AtCommand_FakeName,
	AtCommand_Fireworks,
	AtCommand_Fog,
	AtCommand_Follow,
	AtCommand_GAT,
	AtCommand_GM,
	AtCommand_Gmotd,
	AtCommand_Go,
	AtCommand_Grind,
	AtCommand_Grind2,
	AtCommand_Guild,
	AtCommand_GuildLevelUp,
	AtCommand_GuildRecall,
	AtCommand_GuildSpy,
	AtCommand_GuildStorage,
	AtCommand_GvGOff,
	AtCommand_GvGOn,
	AtCommand_HappyHappyJoyJoy,
	AtCommand_Hatch,
	AtCommand_Hcolor,
	AtCommand_Heal,
	AtCommand_Help,
	AtCommand_Hide,
	AtCommand_Hidenpc,
	AtCommand_Hstyle,
	AtCommand_Identify,
	AtCommand_IDSearch,
	AtCommand_Item,
	AtCommand_ItemCheck,
	AtCommand_ItemInfo,
	AtCommand_ItemList,
	AtCommand_ItemReset,
	AtCommand_Jail,
	AtCommand_JobChange,
	AtCommand_JobLevelUp,
	AtCommand_Jump,
	AtCommand_JumpTo,
	AtCommand_Kami,
	AtCommand_Kick,
	AtCommand_KickAll,
	AtCommand_Kill,
	AtCommand_Killable,
	AtCommand_Killer,
	AtCommand_KillMonster,
	AtCommand_Leaves,
	AtCommand_ListMail,
	AtCommand_ListNewMail,
	AtCommand_Load,
	AtCommand_Loadnpc,
	AtCommand_LocalBroadcast,
	AtCommand_LostSkill,
	AtCommand_MakeEgg,
	AtCommand_MapExit,
	AtCommand_MapFlag,
	AtCommand_MapInfo,
	AtCommand_MapMove,
	AtCommand_Marry,
	AtCommand_Me,
	AtCommand_Memo,
	AtCommand_MiscEffect,
	AtCommand_MobInfo,
	AtCommand_MobSearch,
	AtCommand_Model,
	AtCommand_Monster,
	AtCommand_MonsterBig,
	AtCommand_MonsterIgnore,
	AtCommand_MonsterSmall,
	AtCommand_MountPeco,
	AtCommand_Mute,
	AtCommand_MuteArea,
	AtCommand_Night,
	AtCommand_NpcMove,
	AtCommand_NpcTalk,
	AtCommand_Nuke,
	AtCommand_Option,
	AtCommand_Packet,
	AtCommand_Param,
	AtCommand_Party,
	AtCommand_PartyRecall,
	AtCommand_PartySpy,
	AtCommand_PetFriendly,
	AtCommand_PetHungry,
	AtCommand_PetId,
	AtCommand_PetRename,
	AtCommand_PetTalk,
	AtCommand_PrintStats,
	AtCommand_Produce,
	AtCommand_PvPOff,
	AtCommand_PvPOn,
	AtCommand_QuestSkill,
	AtCommand_Rain,
	AtCommand_Raise,
	AtCommand_RaiseMap,
	AtCommand_Rates,
	AtCommand_ReadMail,
	AtCommand_Recall,
	AtCommand_RecallAll,
	AtCommand_Refine,
	AtCommand_Refresh,
	AtCommand_RefreshOnline,
	AtCommand_ReloadAtcommand,
	AtCommand_ReloadBattleConf,
	AtCommand_ReloadItemDB,
	AtCommand_ReloadMobDB,
	AtCommand_ReloadPcDB,
	AtCommand_ReloadScript,
	AtCommand_ReloadSkillDB,
	AtCommand_ReloadStatusDB,
	AtCommand_RepairAll,
	AtCommand_Revive,
	AtCommand_RuraP,
	AtCommand_Sakura,
	AtCommand_Save,
	AtCommand_Send,
	AtCommand_SendMail,
	AtCommand_ServerTime,
	AtCommand_SetBattleFlag,
	AtCommand_ShowDelay,
	AtCommand_ShowExp,
	AtCommand_Shownpc,
	AtCommand_Shuffle,
	AtCommand_Size,
	AtCommand_Skillid,
	AtCommand_SkillOff,
	AtCommand_SkillOn,
	AtCommand_SkillPoint,
	AtCommand_SkillReset,
	AtCommand_SkillTree,
	AtCommand_Snow,
	AtCommand_Sound,
	AtCommand_Spawn,
	AtCommand_Speed,
	AtCommand_SpiritBall,
	AtCommand_StatAll,
	AtCommand_Status,
	AtCommand_StatusPoint,
	AtCommand_StatusReset,
	AtCommand_Storage,
	AtCommand_StorageList,
	AtCommand_Storeall,
	AtCommand_Summon,
	AtCommand_Trade,
	AtCommand_UnDisguise,
	AtCommand_UndisguiseAll,
	AtCommand_UnJail,
	AtCommand_Unloadnpc,
	AtCommand_UnMute,
	AtCommand_UpTime,
	AtCommand_Users,
	AtCommand_Useskill,
	AtCommand_Version,
	AtCommand_Where,
	AtCommand_Who,
	AtCommand_Who2,
	AtCommand_Who3,
	AtCommand_WhoGM,
	AtCommand_WhoMap,
	AtCommand_WhoMap2,
	AtCommand_WhoMap3,
	AtCommand_WhoZeny,
	AtCommand_Zeny,
	
	AtCommand_Unknown,
	AtCommand_MAX
};

///////////////////////////////////////////////////////////////////////////////
/// function proto
typedef bool (*atcommand_function)(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);

///////////////////////////////////////////////////////////////////////////////
/// stores command entries
struct AtCommandInfo
{
	AtCommandType type;			///< for searching by type
	const char* command;		///< for searching by name
	unsigned char level;		///< required execution level
	unsigned char param;		///< number input parameters (including optional)
	unsigned char option;		///< can process a different char
	// still 1 char left
	atcommand_function proc;	///< function pointer

	static char command_symbol;
};



bool is_atcommand(int fd, struct map_session_data &sd, const char* message, unsigned char gmlvl_override=0);
unsigned char get_atcommand_level(const AtCommandType type);

bool atcommand_config_read(const char *cfgName);



bool atcommand_item(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_mapmove(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_spawn(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_jumpto(int fd, struct map_session_data &sd, const char* command, const CParameterList& param);
bool atcommand_recall(int fd, struct map_session_data &sd, const char* command, const CParameterList& param);
bool atcommand_kickall(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);



#endif

