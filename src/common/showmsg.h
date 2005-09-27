#ifndef _SHOWMSG_H_
#define _SHOWMSG_H_

#include "base.h"

//ok thanks MC I see it
#define SHOW_DEBUG_MSG 1
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

enum msg_type {MSG_NONE,MSG_STATUS,MSG_SQL,MSG_INFORMATION,MSG_CONSOLE,MSG_NOTICE,MSG_WARNING,MSG_DEBUG,MSG_ERROR,MSG_FATALERROR};

extern int _vShowMessage(enum msg_type flag, const char *string, va_list va);
extern int _ShowMessage(enum msg_type flag, const char *string, ...);

#ifdef __GNUC__ 

// direct printf replacement
	#define ShowMessage(string...) _ShowMessage(MSG_NONE,##string)

/* MSG_XX */
	#define ShowMsg(flag,string...) _ShowMessage(flag,##string)
//	#define DisplayMsg(flag,string...) _ShowMessage(flag,##string)
//	#define ShowMessage(flag,string...) _ShowMessage(flag,##string)

/* MSG_STATUS */
	#define ShowStatus(string...) _ShowMessage(MSG_STATUS,##string)
//	#define DisplayStatus(string...) _ShowMessage(MSG_STATUS,##string)

/* MSG_SQL*/
	#define ShowSQL(string...) _ShowMessage(MSG_SQL,##string)
//	#define DisplaySQL(string...) _ShowMessage(MSG_SQL,##string)

/* MSG_INFORMATION */
	#define ShowInfo(string...) _ShowMessage(MSG_INFORMATION,##string)
//	#define DisplayInfo(string...) _ShowMessage(MSG_INFORMATION,##string)
//	#define ShowInformation(string...) _ShowMessage(MSG_INFORMATION,##string)
//	#define DisplayInformation(string...) _ShowMessage(MSG_INFORMATION,##string)

/* MSG_CONSOLE */
	#define ShowConsole(string...) _ShowMessage(MSG_CONSOLE,##string)
//	#define DisplayNotice(string...) _ShowMessage(MSG_NOTICE,##string)

/* MSG_NOTICE */
	#define ShowNotice(string...) _ShowMessage(MSG_NOTICE,##string)
//	#define DisplayNotice(string...) _ShowMessage(MSG_NOTICE,##string)

/* MSG_WARNING */
	#define ShowWarning(string...) _ShowMessage(MSG_WARNING,##string)
//	#define DisplayWarning(string...) _ShowMessage(MSG_WARNING,##string)
//	#define Warn(string...) _ShowMessage(MSG_WARNING,##string)

/* MSG_DEBUG */
	#define ShowDebug(string...) _ShowMessage(MSG_DEBUG,##string)
	#define DisplayDebug(string...) _ShowMessage(MSG_DEBUG,##string)
	#define Debug(string...) _ShowMessage(MSG_DEBUG,##string)
	#define printDebug(string...) _ShowMessage(MSG_DEBUG,##string)

/* MSG_ERROR */
	#define ShowError(string...) _ShowMessage(MSG_ERROR,##string)
//	#define DisplayError(string...) _ShowMessage(MSG_ERROR,##string)
//	#define OutputError(string...) _ShowMessage(MSG_ERROR,##string)

/* MSG_FATALERROR */
	#define ShowFatalError(string...) _ShowMessage(MSG_FATALERROR,##string)
//	#define DisplayFatalError(string...) _ShowMessage(MSG_ERROR,##string)
//	#define Terminate(string...) _ShowMessage(MSG_FATALERROR,##string)
//	#define Kill(string...) _ShowMessage(MSG_FATALERROR,##string)
//	#define AbortEx(string...) _ShowMessage(MSG_FATALERROR,##string)

#else// no __GNUC__

// direct printf replacement
static inline int ShowMessage(const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(MSG_NONE, string, va);
	va_end(va);
	return ret;
}

/* MSG_XX */
static inline int ShowMsg(enum msg_type flag, const char *string, ...)
//	static inline int DisplayMsg(enum msg_type flag, const char *string, ...)
//	static inline int ShowMessage(enum msg_type flag, const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(flag, string, va);
	va_end(va);
	return ret;
}

/* MSG_STATUS */
static inline int ShowStatus(const char *string, ...)
//	static inline int DisplayStatus(const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(MSG_STATUS, string, va);
	va_end(va);
	return ret;
}


/* MSG_SQL*/
static inline int ShowSQL(const char *string, ...)
//	static inline int DisplaySQL(const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(MSG_SQL, string, va);
	va_end(va);
	return ret;
}

/* MSG_INFORMATION */
static inline int ShowInfo(const char *string, ...)
//	static inline int DisplayInfo(const char *string, ...)
//	static inline int ShowInformation(const char *string, ...)
//	static inline int DisplayInformation(const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(MSG_INFORMATION, string, va);
	va_end(va);
	return ret;
}
/* MSG_CONSOLE */
static inline int ShowConsole(const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(MSG_CONSOLE, string, va);
	va_end(va);
	return ret;
}

/* MSG_NOTICE */
static inline int ShowNotice(const char *string, ...)
//	static inline int DisplayNotice(const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(MSG_NOTICE, string, va);
	va_end(va);
	return ret;
}

/* MSG_WARNING */
static inline int ShowWarning(const char *string, ...)
//	static inline int DisplayWarning(const char *string, ...)
//	static inline int Warn(const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(MSG_WARNING, string, va);
	va_end(va);
	return ret;
}

/* MSG_DEBUG */
static inline int ShowDebug(const char *string, ...)
//	static inline int DisplayDebug(const char *string, ...)
//	static inline int Debug(const char *string, ...)
//	static inline int printDebug(const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(MSG_DEBUG, string, va);
	va_end(va);
	return ret;
}

/* MSG_ERROR */
static inline int ShowError(const char *string, ...)
//	static inline int DisplayError(const char *string, ...)
//	static inline int OutputError(const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(MSG_ERROR, string, va);
	va_end(va);
	return ret;
}

/* MSG_FATALERROR */
static inline int ShowFatalError(const char *string, ...)
//	static inline int DisplayFatalError(const char *string, ...)
//	static inline int Terminate(const char *string, ...)
//	static inline int Kill(const char *string, ...)
//	static inline int AbortEx(const char *string, ...)
{
	va_list va;
	int ret;
	va_start(va,string);
	ret = _vShowMessage(MSG_FATALERROR, string, va);
	va_end(va);
	return ret;
}




#endif//__GNUC__


#endif//_SHOWMSG_H_
