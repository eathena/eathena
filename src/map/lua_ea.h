/* LUA_EA module ***Kevin*** lua_ea.h v1.0 */

#ifndef _LUA_EA_H_
#define _LUA_EA_H_

#define LUA_SCRIPTS_NAME "script/lua_main.conf"

struct lua_ea_src_list {
	struct lua_ea_src_list *next;
	char file[4];
};

struct LuaCommandInfo {
	const char *command;
	lua_CFunction f;
};
typedef enum {
	NRUN, // Script is ready
	NEXT, // Waiting for the player to click [Next]
	CLOSE, // Waiting for the player to click [Close]
	MENU, // Waiting for the player to choose a menu option
	INPUT, // Waiting for the player to input a value
	INPUT_STR, // Waiting for the player to input a string
	LUA_SHOP, // Waiting for the player to choose [Buy] or [Sell]
	BUY, // Waiting for the player to choose items to buy
	SELL // Waiting for the player to choose items to sell
} lua_script_state;

typedef enum {
	NONE,
	NPC,
	AREASCRIPT
} lua_npc_type;

struct lua_ea_player_data {
	int script_id;
	lua_State *NL;
	lua_script_state state;
};

struct lua_ea_npc_data {
	lua_npc_type type;
	char function[50];
	
	//areascript data
	struct ad {
		int x1;
		int y1;
		int x2;
		int y2;
	} ad;
};

/*init/close lua module*/
void init_lua_ea(void);
void close_lua_ea(void);

/*lua system functions*/
void lua_ea_buildin_commands();
int lua_ea_npc_add(char *name,char *exname,int m,int x,int y,int dir,int class_,char *function);
int lua_ea_areascript_add(char *name,int m,int x1,int y1,int x2,int y2,char *function);
int lua_ea_load_scripts(const char *srcName);
int lua_ea_script_file_add(char* file);
void lua_ea_load_script_files();
int lua_ea_get_new_npc_id(void);

/*script control*/
int lua_ea_continue(struct map_session_data *sd, char *format, ...);
void lua_ea_run_function(const char *name,int char_id,const char *format,...);
void lua_ea_run_chunk(const char *chunk,int char_id);
void lua_ea_close_script(struct map_session_data *sd);

/*script related functions*/
struct map_session_data * lua_ea_get_target(lua_State *NL,int idx);
void lua_ea_resume(struct map_session_data *sd,const char *format,...);
int lua_ea_sendbuylist(struct map_session_data *sd);
int lua_ea_run(struct map_session_data *sd, struct npc_data *nd);

#endif
