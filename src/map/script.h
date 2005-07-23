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
	char die_event_name[24];
	char kill_event_name[24];
	char login_event_name[24];
	char logout_event_name[24];
	char mapload_event_name[24];
	unsigned event_requires_trigger : 1;
} script_config;

struct script_data {
	int type;
	union {
		int num;
		const char *str;
	} u;
};

struct script_stack {
	int sp;
	size_t sp_max;
	struct script_data *stack_data;
	script_stack() : sp(0),sp_max(0),stack_data(NULL)	{}
};
struct script_state
{
	struct script_stack stack;
	size_t start;
	size_t end;
	size_t pos;
	int state;
	unsigned long rid;
	unsigned long oid;
	const char *script;
	const char *new_script;
	int defsp;
	int new_pos;
	int new_defsp;
};

char *parse_script(unsigned char *src,int line);
int run_script(const char *script,int pos,int rid,int oid);

int set_var(const char *name, void *v);
int conv_num(struct script_state *st,struct script_data *data);
const char* conv_str(struct script_state &st,struct script_data &data);

struct dbt* script_get_label_db();
struct dbt* script_get_userfunc_db();

int script_config_read(const char *cfgName);
int do_init_script();
int do_final_script();

extern char mapreg_txt[];

#endif

