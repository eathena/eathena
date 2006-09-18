// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#ifndef _ATCOMMAND_H_
#define _ATCOMMAND_H_

#include "baseparam.h"

///////////////////////////////////////////////////////////////////////////////
/// predeclaration
struct map_session_data;


///////////////////////////////////////////////////////////////////////////////
/// function proto
typedef bool (*command_function)(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);


///////////////////////////////////////////////////////////////////////////////
/// stores command entries
struct CommandInfo
{
	const char*			command;	///< for searching by name
	unsigned char		level;		///< required execution level
	unsigned char		param;		///< number input parameters (including optional)
	unsigned char		option;		///< can process a different char
									// still 1 char left until aligned
	command_function	func;		///< function pointer

	static char command_symbol;


	/// converts name or id to sd.
	static map_session_data *param2sd(const char* str);
	/// checks and executes a command.
	static bool is_command(int fd, map_session_data &sd, const char* message, unsigned char gmlvl_override=0);
	/// returns command requirement level.
	static unsigned char get_level(command_function func);
	/// returns command info by command name.
	static CommandInfo& byname(const char* name);
	/// read configuration file.
	static bool config_read(const char *cfgName);
};





// externally used commands
bool command_item(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);
bool command_mapmove(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);
bool command_spawn(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);
bool command_jumpto(int fd, map_session_data &sd, const char* command, const basics::CParameterList& param);
bool command_recall(int fd, map_session_data &sd, const char* command, const basics::CParameterList& param);
bool command_kickall(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);
bool command_hide(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);
bool command_mute(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);
bool command_kick(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);
bool command_broadcast(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);
bool command_localbroadcast(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);
bool command_statusreset(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);
bool command_skillreset(int fd, map_session_data& sd, const char* command, const basics::CParameterList& param);



#endif

