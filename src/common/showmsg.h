// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _SHOWMSG_H_
#define _SHOWMSG_H_

#include "basetypes.h"

// for help with the console colors look here:
// http://www.edoceo.com/liberum/?doc=printf-with-color
// some code explanation (used here):
// \033[2J : clear screen and go up/left (0, 0 position)
// \033[K  : clear line from actual position to end of the line
// \033[0m : reset color parameter
// \033[1m : use bold for font

#define	CL_RESET		"\033[0m"
#define CL_CLS			"\033[2J"
#define CL_CLL			"\033[K"

// font settings
#define	CL_BOLD			"\033[1m"
#define	CL_STD			"\033[0m"
// foreground color
#define	CL_BLACK		"\033[30m"
#define	CL_RED			"\033[31m"
#define	CL_GREEN		"\033[32m"
#define	CL_YELLOW		"\033[33m"
#define	CL_BLUE			"\033[34m"
#define	CL_MAGENTA		"\033[35m"
#define	CL_CYAN			"\033[36m"
#define	CL_WHITE		"\033[37m"
// background color
#define	CL_BG_BLACK		"\033[40m"
#define	CL_BG_RED		"\033[41m"
#define	CL_BG_GREEN		"\033[42m"
#define	CL_BG_YELLOW	"\033[43m"
#define	CL_BG_BLUE		"\033[44m"
#define	CL_BG_MAGENTA	"\033[45m"
#define	CL_BG_CYAN		"\033[46m"
#define	CL_BG_WHITE		"\033[47m"

// foreground color and normal font (normal color on windows)
#define	CL_LT_BLACK		"\033[0;30m"
#define	CL_LT_RED		"\033[0;31m"
#define	CL_LT_GREEN		"\033[0;32m"
#define	CL_LT_YELLOW	"\033[0;33m"
#define	CL_LT_BLUE		"\033[0;34m"
#define	CL_LT_MAGENTA	"\033[0;35m"
#define	CL_LT_CYAN		"\033[0;36m"
#define	CL_LT_WHITE		"\033[0;37m"
// foreground color and bold font (bright color on windows)
#define	CL_BT_BLACK		"\033[1;30m"
#define	CL_BT_RED		"\033[1;31m"
#define	CL_BT_GREEN		"\033[1;32m"
#define	CL_BT_YELLOW	"\033[1;33m"
#define	CL_BT_BLUE		"\033[1;34m"
#define	CL_BT_MAGENTA	"\033[1;35m"
#define	CL_BT_CYAN		"\033[1;36m"
#define	CL_BT_WHITE		"\033[1;37m"

#define	CL_WTBL			"\033[37;44m"	// white on blue
#define	CL_XXBL			"\033[0;44m"	// default on blue
#define CL_PASS			"\033[0;32;42m"	// green on green

#define	CL_NORM		CL_RESET
#define CL_NORMAL	CL_RESET
#define CL_NONE		CL_RESET

#define CL_SPACE         "           "	// space aquivalent of the print messages
enum msg_type {MSG_NONE,MSG_STATUS,MSG_SQL,MSG_INFORMATION,MSG_CONSOLE,MSG_NOTICE,MSG_WARNING,MSG_DEBUG,MSG_ERROR,MSG_FATALERROR};

extern int _vShowMessage(enum msg_type flag, const char *str, va_list va);
extern int _ShowMessage(enum msg_type flag, const char *str, ...);

#ifdef __GNUC__ 

// direct printf replacement
	#define ShowMessage(str...) _ShowMessage(MSG_NONE,##str)

// MSG_XX
	#define ShowMsg(flag,str...) _ShowMessage(flag,##str)
//	#define DisplayMsg(flag,str...) _ShowMessage(flag,##str)
//	#define ShowMessage(flag,str...) _ShowMessage(flag,##str)

// MSG_STATUS
	#define ShowStatus(str...) _ShowMessage(MSG_STATUS,##str)
//	#define DisplayStatus(str...) _ShowMessage(MSG_STATUS,##str)

// MSG_SQL
	#define ShowSQL(str...) _ShowMessage(MSG_SQL,##str)
//	#define DisplaySQL(str...) _ShowMessage(MSG_SQL,##str)

// MSG_INFORMATION
	#define ShowInfo(str...) _ShowMessage(MSG_INFORMATION,##str)
//	#define DisplayInfo(str...) _ShowMessage(MSG_INFORMATION,##str)
//	#define ShowInformation(str...) _ShowMessage(MSG_INFORMATION,##str)
//	#define DisplayInformation(str...) _ShowMessage(MSG_INFORMATION,##str)

// MSG_CONSOLE
	#define ShowConsole(str...) _ShowMessage(MSG_CONSOLE,##str)
//	#define DisplayNotice(str...) _ShowMessage(MSG_NOTICE,##str)

// MSG_NOTICE
	#define ShowNotice(str...) _ShowMessage(MSG_NOTICE,##str)
//	#define DisplayNotice(str...) _ShowMessage(MSG_NOTICE,##str)

// MSG_WARNING
	#define ShowWarning(str...) _ShowMessage(MSG_WARNING,##str)
//	#define DisplayWarning(str...) _ShowMessage(MSG_WARNING,##str)
//	#define Warn(str...) _ShowMessage(MSG_WARNING,##str)

// MSG_DEBUG
	#define ShowDebug(str...) _ShowMessage(MSG_DEBUG,##str)
	#define DisplayDebug(str...) _ShowMessage(MSG_DEBUG,##str)
	#define Debug(str...) _ShowMessage(MSG_DEBUG,##str)
	#define printDebug(str...) _ShowMessage(MSG_DEBUG,##str)

// MSG_ERROR
	#define ShowError(str...) _ShowMessage(MSG_ERROR,##str)
//	#define DisplayError(str...) _ShowMessage(MSG_ERROR,##str)
//	#define OutputError(str...) _ShowMessage(MSG_ERROR,##str)

// MSG_FATALERROR
	#define ShowFatalError(str...) _ShowMessage(MSG_FATALERROR,##str)
//	#define DisplayFatalError(str...) _ShowMessage(MSG_ERROR,##str)
//	#define Terminate(str...) _ShowMessage(MSG_FATALERROR,##str)
//	#define Kill(str...) _ShowMessage(MSG_FATALERROR,##str)
//	#define AbortEx(str...) _ShowMessage(MSG_FATALERROR,##str)

#else// no __GNUC__

// direct printf replacement
static inline int ShowMessage(const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(MSG_NONE, str, va);
	va_end(va);
	return ret;
}

// MSG_XX
static inline int ShowMsg(enum msg_type flag, const char *str, ...)
//	static inline int DisplayMsg(enum msg_type flag, const char *str, ...)
//	static inline int ShowMessage(enum msg_type flag, const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(flag, str, va);
	va_end(va);
	return ret;
}

// MSG_STATUS
static inline int ShowStatus(const char *str, ...)
//	static inline int DisplayStatus(const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(MSG_STATUS, str, va);
	va_end(va);
	return ret;
}


// MSG_SQL
static inline int ShowSQL(const char *str, ...)
//	static inline int DisplaySQL(const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(MSG_SQL, str, va);
	va_end(va);
	return ret;
}

// MSG_INFORMATION
static inline int ShowInfo(const char *str, ...)
//	static inline int DisplayInfo(const char *str, ...)
//	static inline int ShowInformation(const char *str, ...)
//	static inline int DisplayInformation(const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(MSG_INFORMATION, str, va);
	va_end(va);
	return ret;
}
// MSG_CONSOLE
static inline int ShowConsole(const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(MSG_CONSOLE, str, va);
	va_end(va);
	return ret;
}

// MSG_NOTICE
static inline int ShowNotice(const char *str, ...)
//	static inline int DisplayNotice(const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(MSG_NOTICE, str, va);
	va_end(va);
	return ret;
}

// MSG_WARNING
static inline int ShowWarning(const char *str, ...)
//	static inline int DisplayWarning(const char *str, ...)
//	static inline int Warn(const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(MSG_WARNING, str, va);
	va_end(va);
	return ret;
}

// MSG_DEBUG
static inline int ShowDebug(const char *str, ...)
//	static inline int DisplayDebug(const char *str, ...)
//	static inline int Debug(const char *str, ...)
//	static inline int printDebug(const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(MSG_DEBUG, str, va);
	va_end(va);
	return ret;
}

// MSG_ERROR
static inline int ShowError(const char *str, ...)
//	static inline int DisplayError(const char *str, ...)
//	static inline int OutputError(const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(MSG_ERROR, str, va);
	va_end(va);
	return ret;
}

// MSG_FATALERROR
static inline int ShowFatalError(const char *str, ...)
//	static inline int DisplayFatalError(const char *str, ...)
//	static inline int Terminate(const char *str, ...)
//	static inline int Kill(const char *str, ...)
//	static inline int AbortEx(const char *str, ...)
{
	va_list va;
	int ret;
	va_start(va,str);
	ret = _vShowMessage(MSG_FATALERROR, str, va);
	va_end(va);
	return ret;
}




#endif//__GNUC__


#endif//_SHOWMSG_H_
