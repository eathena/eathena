#pragma once

#define SHOW_DEBUG_MSG 1
#define	CL_RESET	"\033[0;0m"
#define CL_NORMAL	CL_RESET
#define CL_NONE		CL_RESET
#define	CL_WHITE	"\033[1;29m"
#define	CL_GRAY		"\033[1;30m"
#define	CL_RED		"\033[1;31m"
#define	CL_GREEN	"\033[1;32m"
#define	CL_YELLOW	"\033[1;33m"
#define	CL_BLUE		"\033[1;34m"
#define	CL_MAGENTA	"\033[1;35m"
#define	CL_CYAN		"\033[1;36m"

enum msg_type {
MSG_STATUS, MSG_SQL, MSG_INFORMATION,MSG_NOTICE,
MSG_WARNING,MSG_DEBUG,MSG_ERROR,MSG_FATALERROR
};

class ShowMessage {
	public:
		// Constructors
		ShowMsg(void);
		ShowMsg(const char *string);
		ShowMsg(const char *string, enum msg_type flag);

		int	_ShowMessage(const char *string, enum msg_type flag);

		inline int	ShowMsg(const char *string, enum msg_type flag) { return _ShowMessage(string, flag); };
		inline int	ShowStatus(const char *string) { return _ShowMessage(string, MSG_STATUS); };
		inline int	ShowSQL(const char *string) { return _ShowMessage(string, MSG_SQL); };
		inline int	ShowInfo(const char *string) { return _ShowMessage(string, MSG_INFORMATION); };
		inline int	ShowNotice(const char *string) { return _ShowMessage(string, MSG_NOTICE); };
		inline int	ShowWarning(const char *string) { return _ShowMessage(string, MSG_WARNING); };
		inline int	ShowDebug(const char *string) { return _ShowMessage(string, MSG_DEBUG); };
		inline int	ShowError(const char *string) { return _ShowMessage(string, MSG_ERROR); };
		inline int	ShowFatalError(const char *string) { return _ShowMessage(string, MSG_FATALERROR); };

		char tmp_output[1024];
};

extern class ShowMessage ShowMsg;