// $Id: script.h,v 1.2 2004/09/25 05:32:19 MouseJstr Exp $
#ifndef _SCRIPT_H_
#define _SCRIPT_H_

extern struct Script_Config {
	unsigned verbose_mode : 1;
	unsigned warn_func_no_comma : 1;
	unsigned warn_cmd_no_comma : 1;
	unsigned warn_func_mismatch_paramnum : 1;
	unsigned warn_cmd_mismatch_paramnum : 1;
	int check_cmdcount;
	int check_gotocount;

	unsigned event_script_type : 1;
	char* die_event_name;
	char* kill_event_name;
	char* login_event_name;
	char* logout_event_name;
	char* mapload_event_name;
	unsigned event_requires_trigger : 1;
} script_config;

struct LuaCommandInfo {
	const char *command;
	lua_CFunction f;
};

enum state {
	RUNNING,
	HALT,
	STOP,
	NRUN
};

state script_state=NRUN;

extern char mapreg_txt[];

void script_run_function(const char *name,const char *format,...); // [DracoRPG]
void script_run_chunk(const char *chunk); // [DracoRPG]
void script_buildin_commands(); // [Kevin]
int script_config_read(char *cfgName);
int do_init_script();
int do_final_script();

#endif
