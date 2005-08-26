#ifndef _CHARCOMMAND_H_
#define _CHARCOMMAND_H_

enum CharCommandType {
	CharCommand_None = -1,
	CharCommandJobChange,
	CharCommandPetRename,
	CharCommandPetFriendly,
	CharCommandReset,
	CharCommandStats,
	CharCommandOption,
	CharCommandSave,
	CharCommandStatsAll,
	CharCommandSpiritball,
	CharCommandItemList,
	CharCommandEffect,
	CharCommandStorageList,
	CharCommandItem, // by MC Cameri
	CharCommandWarp,
	CharCommandZeny,
	CharCommandShowExp,
	CharCommandShowDelay,

	CharCommandFakeName,
	CharCommandBaseLevel,
	CharCommandJobLevel,
	CharCommandQuestSkill,
	CharCommandLostSkill,
	CharCommandSkReset,
	CharCommandStReset,
	CharCommandModel,
	CharCommandSKPoint,
	CharCommandSTPoint,
	CharCommandChangeSex,

#ifdef TXT_ONLY
/* TXT_ONLY */

/* TXT_ONLY */
#else
/* SQL-only */

/* SQL Only */
#endif
	
	// End. No more commans after this line.
	CharCommand_Unknown,
	CharCommand_MAX
};

typedef enum CharCommandType CharCommandType;

typedef struct CharCommandInfo {
	CharCommandType type;
	const char* command;
	unsigned char level;
	bool (*proc)(int fd, struct map_session_data &sd, const char* command, const char* message);
} CharCommandInfo;

CharCommandType is_charcommand(int fd, struct map_session_data &sd, const char* message, unsigned char gmlvl);
unsigned char get_charcommand_level(const CharCommandType type);

bool charcommand_config_read(const char *cfgName);

#endif

