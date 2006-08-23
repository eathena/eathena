
#ifndef _ATCOMMAND_H_
#define _ATCOMMAND_H_

#include "utils.h"

///////////////////////////////////////////////////////////////////////////////
/// function proto
typedef bool (*atcommand_function)(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);

///////////////////////////////////////////////////////////////////////////////
/// stores command entries
struct AtCommandInfo
{
	const char*			command;	///< for searching by name
	unsigned char		level;		///< required execution level
	unsigned char		param;		///< number input parameters (including optional)
	unsigned char		option;		///< can process a different char
									// still 1 char left until aligned
	atcommand_function	func;		///< function pointer

	static char command_symbol;
};



bool is_atcommand(int fd, struct map_session_data &sd, const char* message, unsigned char gmlvl_override=0);
unsigned char get_atcommand_level(atcommand_function func);

bool atcommand_config_read(const char *cfgName);



bool atcommand_item(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_mapmove(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_spawn(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_jumpto(int fd, struct map_session_data &sd, const char* command, const CParameterList& param);
bool atcommand_recall(int fd, struct map_session_data &sd, const char* command, const CParameterList& param);
bool atcommand_kickall(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_hide(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_mute(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_kick(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_broadcast(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_localbroadcast(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_statusreset(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);
bool atcommand_skillreset(int fd, struct map_session_data& sd, const char* command, const CParameterList& param);



#endif

