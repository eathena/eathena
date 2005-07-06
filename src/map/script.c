// $Id: script.c 148 2004-09-30 14:05:37Z MouseJstr $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <time.h>

#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/lock.h"
#include "../common/db.h"
#include "../common/nullpo.h"

#include "map.h"
#include "clif.h"
#include "chrif.h"
#include "itemdb.h"
#include "pc.h"
#include "status.h"
#include "script.h"
#include "storage.h"
#include "mob.h"
#include "npc.h"
#include "pet.h"
#include "intif.h"
#include "skill.h"
#include "chat.h"
#include "battle.h"
#include "party.h"
#include "guild.h"
#include "atcommand.h"
#include "log.h"
#include "showmsg.h"

//static struct dbt *mapreg_db=NULL;
//static struct dbt *mapregstr_db=NULL;
//static int mapreg_dirty=-1;
char mapreg_txt[256]="save/mapreg.txt";
//#define MAPREG_AUTOSAVE_INTERVAL	(10*1000)

struct Script_Config script_config;
static char pos[11][100] = {"Head","Body","Left hand","Right hand","Robe","Shoes","Accessory 1","Accessory 2","Head 2","Head 3","Not Equipped"};

lua_State *L;


/*===================================================================
 *
 *                  LUA BUILD-IN COMMANDS GO HERE
 *
 *===================================================================
 */

// scriptend()
// Exits definitively the script
static int buildin_scriptend(lua_State *NL)
{
	return lua_yield(NL, 0);
}

// addnpc("NPC name","name postfix","map.gat",x,y,dir,sprite,"function")
// Add an standard NPC that triggers a function when clicked
static int buildin_addnpc(lua_State *NL)
{
	char name[24],exname[24],map[16],function[50];
	short m,x,y,dir,class_;
	
	sprintf(name, "%s", lua_tostring(NL,1));
	sprintf(exname, "%s", lua_tostring(NL,2));
	sprintf(map, "%s", lua_tostring(NL,3));
	m=map_mapname2mapid(map);
	x=lua_tonumber(NL,4);
	y=lua_tonumber(NL,5);
	dir=lua_tonumber(NL,6);
	class_=lua_tonumber(NL,7);
	sprintf(function, "%s", lua_tostring(NL,8));
	
	npc_add(name,exname,m,x,y,dir,class_,function);

	return 0;
}

// addareascript("Area script name","map.gat",x1,y1,x2,y2,"function")
// Add an invisible area that triggers a function when entered
static int buildin_addareascript(lua_State *NL)
{
	char name[24],map[16],function[50];
	short m,x1,y1,x2,y2;

	sprintf(name,"%s",lua_tostring(NL,1));
	sprintf(map,"%s",lua_tostring(NL,2));
	m=map_mapname2mapid(map);
	x1=(lua_tonumber(NL,5)>lua_tonumber(NL,3))?lua_tonumber(NL,3):lua_tonumber(NL,5);
	y1=(lua_tonumber(NL,6)>lua_tonumber(NL,4))?lua_tonumber(NL,4):lua_tonumber(NL,6);
	x2=(lua_tonumber(NL,5)>lua_tonumber(NL,3))?lua_tonumber(NL,5):lua_tonumber(NL,3);
	y2=(lua_tonumber(NL,6)>lua_tonumber(NL,4))?lua_tonumber(NL,6):lua_tonumber(NL,4);
	sprintf(function,"%s",lua_tostring(NL,7));

	areascript_add(name,m,x1,y1,x2,y2,function);

	return 0;
}

// addwarp("Warp name","map.gat",x,y,"destmap.gat",destx,desty,xradius,yradius)
// Add a warp that moves players to somewhere else when entered
static int buildin_addwarp(lua_State *NL)
{
	char name[24],map[16],destmap[16];
	short m,x,y;
	short destx,desty,xs,ys;
	
	sprintf(name,"%s",lua_tostring(NL,1));
	sprintf(map,"%s",lua_tostring(NL,2));
	m=map_mapname2mapid(map);
	x=lua_tonumber(NL,3);
	y=lua_tonumber(NL,4);
	sprintf(destmap,"%s",lua_tostring(NL,5));
	destx=lua_tonumber(NL,6);
	desty=lua_tonumber(NL,7);
	xs=lua_tonumber(NL,8);
	ys=lua_tonumber(NL,9);

	warp_add(name,m,x,y,destmap,destx,desty,xs,ys);

	return 0;
}

// addspawn("Monster name","map.gat",x,y,xradius,yradius,id,amount,delay1,delay2,"function")
// Add a monster spawn etc
static int buildin_addspawn(lua_State *NL)
{
	char name[24],map[16],function[50];
	short m,x,y,xs,ys,class_,num,d1,d2;

	sprintf(name,"%s",lua_tostring(NL,1));
	sprintf(map,"%s",lua_tostring(NL,2));
	m=map_mapname2mapid(map);
	x=lua_tonumber(NL,3);
	y=lua_tonumber(NL,4);
	xs=lua_tonumber(NL,5);
	ys=lua_tonumber(NL,6);
	class_=lua_tonumber(NL,7);
	num=lua_tonumber(NL,8);
	d1=lua_tonumber(NL,9);
	d2=lua_tonumber(NL,10);
	sprintf(function,"%s",lua_tostring(NL,11));

	spawn_add(name,m,x,y,xs,ys,class_,num,d1,d2,function);

	return 0;
}

// npcmes("Text",[id])
// Print the text into the NPC dialog window of the player
static int buildin_npcmes(lua_State *NL)
{
	struct map_session_data *sd;
	char mes[512];

	sprintf(mes,"%s",lua_tostring(NL, 1)); 
	sd = script_get_target(NL, 2);

	clif_scriptmes(sd, sd->npc_id, mes);

	return 0;
}

// npcclose([id])
// Display a [Close] button in the NPC dialog window of the player and pause the script until the button is clicked
static int buildin_npcclose(lua_State *NL)
{
	struct map_session_data *sd;

	sd = script_get_target(NL, 1);

	clif_scriptclose(sd,sd->npc_id);

	sd->script_state = CLOSE;
	return lua_yield(NL, 0);
}

// npcnext([id])
// Display a [Next] button in the NPC dialog window of the player and pause the script until the button is clicked
static int buildin_npcnext(lua_State *NL)
{
	struct map_session_data *sd;

	sd = script_get_target(NL, 1);

	clif_scriptnext(sd,sd->npc_id);

	sd->script_state = NEXT;
	return lua_yield(NL, 0);
}

// npcinput(type,[id])
// Display an NPC input window asking the player for a value
static int buildin_npcinput(lua_State *NL)
{
	struct map_session_data *sd;
	int type;

	type = lua_tonumber(NL, 1);
	sd = script_get_target(NL, 2);

	switch(type){
		case 0:
			clif_scriptinput(sd,sd->npc_id);
			break;
		case 1:
			clif_scriptinputstr(sd,sd->npc_id);
			break;
	}
	
	sd->script_state = INPUT;
	return lua_yield(NL, 1);
}

// npcmenu("menu_name1",return_value1,...)
// Display an NPC input window asking the player for a value
static int buildin_npcmenu(lua_State *NL)
{
	struct map_session_data *sd;
	char *buf;
	char *menu;
	int len=0, n, i;

	sd = script_get_target(NL, 2);

	lua_pushliteral(NL, "n");
	lua_rawget(NL, 1);
	n = lua_tonumber(NL, -1);
	lua_pop(NL, 1);

	if(n%2 == 1) {
		lua_pushstring(NL, "Incorrect number of arguments for function 'npcmenu'\n");
		lua_error(NL);
		return -1;
	}

	if(!sd->npc_menu_data.current) {
		sd->npc_menu_data.current=0;
	}

	for(i=0; i<n; i+=2) {
		lua_pushnumber(NL, i+1);
		lua_rawget(NL, 1);
		menu = (char *)lua_tostring(NL, -1);
		lua_pop(NL, 1);
		len += strlen(menu);
	}

	buf=(char *)aCallocA(len+1, sizeof(char));
	buf[0]=0;

	for(i=0; i<n; i+=2) {
		lua_pushnumber(NL, i+1);
		lua_rawget(NL, 1);
		menu = (char *)lua_tostring(NL, -1);
		lua_pop(NL, 1);

		lua_pushnumber(NL, i+2);
		lua_rawget(NL, 1);
		sd->npc_menu_data.value[sd->npc_menu_data.current] = lua_tonumber(NL, -1);
		lua_pop(NL, 1);

		sd->npc_menu_data.id[sd->npc_menu_data.current] = sd->npc_menu_data.current;
		sd->npc_menu_data.current+=1;
		
		strcat(buf,menu);
		strcat(buf,":");
	}

	clif_scriptmenu(sd,sd->npc_id,buf);
	aFree(buf);

	sd->script_state = MENU;
	return lua_yield(NL, 1);
}

// npcshop(item_id1,item_price1,...)
// Start a shop with buylist of item_id selling for item_price
static int buildin_npcshop(lua_State *NL)
{
	struct map_session_data *sd;
	int n, i, j;

	sd = script_get_target(NL, 2);

	lua_pushliteral(NL, "n");
	lua_rawget(NL, 1);
	n = lua_tonumber(NL, -1);
	lua_pop(NL, 1);

	if(n%2 == 1) {
		lua_pushstring(NL, "Incorrect number of arguments for function 'npcmenu'\n");
		lua_error(NL);
		return -1;
	}

	sd->shop_data.n = n/2;

	for(i=1, j=0; i<=n; i+=2, j++) {
		lua_pushnumber(NL, i);
		lua_rawget(NL, 1);
		sd->shop_data.nameid[j] = lua_tonumber(NL, -1);
		lua_pop(NL, 1);

		lua_pushnumber(NL, i+1);
		lua_rawget(NL, 1);
		sd->shop_data.value[j] = lua_tonumber(NL, -1);
		lua_pop(NL, 1);
	}

	clif_npcbuysell(sd, sd->npc_id);

	sd->script_state = SHOP;
	return lua_yield(NL, 1);
}

// npccutin(name,type,[id])
// Display a cutin picture on the screen
static int buildin_npccutin(lua_State *NL)
{
	struct map_session_data *sd;
	char name[50];
	int type;

	sprintf(name, "%s", lua_tostring(NL,1));
	type = lua_tonumber(NL, 2);
	sd = script_get_target(NL, 3);

	clif_cutin(sd,name,type);

	return 0;
}

// heal(hp,sp,[id])
// Heal the character by a set amount of HP and SP
static int buildin_heal(lua_State *NL)
{
	struct map_session_data *sd;
	int hp, sp;

	hp = lua_tonumber(NL, 1);
	sp = lua_tonumber(NL, 2);
	sd = script_get_target(NL, 3);

	pc_heal(sd, hp, sp);

	return 0;
}

// percentheal(hp,sp,[id])
// Heal the character by a percentage of MaxHP and MaxSP
static int buildin_percentheal(lua_State *NL)
{
	struct map_session_data *sd;
	int hp, sp;

	hp = lua_tonumber(NL, 1);
	sp = lua_tonumber(NL, 2);
	sd = script_get_target(NL, 3);

	pc_percentheal(sd, hp, sp);

	return 0;
}

// itemheal(hp,sp,[id])
// Heal the character by an amount of HP and SP that increases with VIT/INT, skills, etc
static int buildin_itemheal(lua_State *NL)
{
	struct map_session_data *sd;
	int hp, sp;

	hp = lua_tonumber(NL, 1);
	sp = lua_tonumber(NL, 2);
	sd = script_get_target(NL, 3);

	pc_itemheal(sd, hp, sp);

	return 0;
}

// warp("map.gat",x,y,[id])
// Warp the character to a set location
static int buildin_warp(lua_State *NL)
{
	struct map_session_data *sd;
	char str[16];
	int x, y;

	sprintf(str, "%s", lua_tostring(NL,1));
	x = lua_tonumber(NL, 2);
	y = lua_tonumber(NL, 3);
	sd = script_get_target(NL, 4);

	if(strcmp(str,"Random")==0) // Warp to random location
		pc_randomwarp(sd,3);
	else if(strcmp(str,"SavePoint")==0 || strcmp(str,"Save")==0) { // Warp to save point
		if(map[sd->bl.m].flag.noreturn)
			return 0;
		pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);
	} else // Warp to given location
		pc_setpos(sd,str,x,y,0);

	return 0;
}

// jobchange(job,[id])
// Change the job of the character
static int buildin_jobchange(lua_State *NL)
{
	struct map_session_data *sd;
	int job;

	job = lua_tonumber(NL, 1);
	sd = script_get_target(NL, 2);

	pc_jobchange(sd,job,0);

	return 0;
}

// setlook(type,val,[id])
// Change the look of the character
static int buildin_setlook(lua_State *NL)
{
	struct map_session_data *sd;
	int type,val;

	type = lua_tonumber(NL, 1);
	val = lua_tonumber(NL, 2);
	sd = script_get_target(NL, 3);

	pc_changelook(sd,type,val);

	return 0;
}

// List of commands to build into Lua, format : {"function_name_in_lua", C_function_name}
static struct LuaCommandInfo commands[] = {
	// Basic functions
	{"scriptend", buildin_scriptend},
	// Object creation functions
	{"addnpc", buildin_addnpc},
	{"addareascript", buildin_addareascript},
	{"addwarp", buildin_addwarp},
	{"addspawn", buildin_addspawn},
//	{"addgmcommand", buildin_addgmcommand},
//	{"addtimer", buildin_addtimer},
//  {"addclock", buildin_addclock},
//	{"addevent", buildin_addevent},
	// NPC dialog functions
	{"npcmes", buildin_npcmes},
	{"npcclose", buildin_npcclose},
	{"npcnext", buildin_npcnext},
	{"npcinput", buildin_npcinput},
	{"npcmenu_", buildin_npcmenu},
	{"npcshop_", buildin_npcshop},
	{"npccutin", buildin_npccutin},
	// Player related functions
	{"heal", buildin_heal},
	{"percentheal", buildin_percentheal},
	{"itemheal", buildin_itemheal},
	{"warp", buildin_warp},
	{"jobchange", buildin_jobchange},
	{"setlook", buildin_setlook},
	// End of build-in functions list
	{"-End of list-", NULL},
};

/*===================================================================
 *
 *                   END OF LUA BUILD-IN COMMANDS
 *
 *===================================================================
 */

// Register build-in commands specified above
void script_buildin_commands()
{
	int i=0;

	while(commands[i].f) {
		lua_pushstring(L, commands[i].command);
        lua_pushcfunction(L, commands[i].f);
        lua_settable(L, LUA_GLOBALSINDEX);
        i++;
    }
	ShowStatus("Done registering '"CL_WHITE"%d"CL_RESET"' script build-in commands.\n",i);
}

// Check whether a char ID was passed as argument, else check if there's a global one
struct map_session_data* script_get_target(lua_State *NL,int idx)
{
	int char_id;
	struct map_session_data* sd;

	if((char_id=lua_tonumber(NL, idx))==0) { // If 0 or nothing was passed as argument
		lua_pushliteral(NL, "char_id");
		lua_rawget(NL, LUA_GLOBALSINDEX);
		char_id=lua_tonumber(NL, -1); // Get the thread's char ID if it's a personal one
		lua_pop(NL, 1);
	}

	if(char_id==0 || (sd=map_charid2sd(char_id))==NULL) { // If we still dont have a valid char ID here, there's a problem
		ShowError("Target character not found for script command\n");
		return NULL;
	}
	
	return sd;
}

// Run a Lua function that was previously loaded, specifying the type of arguments with a "format" string
void script_run_function(const char *name,int char_id,const char *format,...)
{
	va_list arg;
	lua_State *NL;
	struct map_session_data *sd=NULL;
	int n=0;

	if (char_id == 0) { // If char_id points to no player
		NL = L; // Use the global thread
	} else { // Else we want to run the function for a specific player
		sd = map_charid2sd(char_id);
		nullpo_retv(sd);
		if(sd->script_state!=NRUN) { // Check that the player is not currently running a script
			ShowError("Cannot run function %s for player %d : player is already running a script\n",name,char_id);
			return;
		}
		NL = sd->NL = lua_newthread(L); // Use the player's personal thread
		lua_pushliteral(NL,"char_id"); // Push global key for char_id
		lua_pushnumber(NL,char_id); // Push value for char_id
		lua_rawset(NL,LUA_GLOBALSINDEX); // Tell Lua to set char_id as a global var
	}

	lua_getglobal(NL,name); // Pass function name to Lua
 
	va_start(arg,format); // Initialize the argument list
	while (*format) { // Pass arguments to Lua, according to the types defined by "format"
        switch (*format++) {
          case 'd': // d = Double
            lua_pushnumber(NL,va_arg(arg,double));
            break;
          case 'i': // i = Integer
            lua_pushnumber(NL,va_arg(arg,int));
            break;
          case 's': // s = String
            lua_pushstring(NL,va_arg(arg,char*));
            break;
          default: // Unknown code
            ShowError("%c : Invalid argument type code, allowed codes are 'd'/'i'/'s'\n",*(format-1));
        }
        n++;
        luaL_checkstack(NL,1,"Too many arguments");
    }

	va_end(arg);

	if (lua_resume(NL,n)!=0) { // Tell Lua to run the function
		ShowError("Cannot run function %s : %s\n",name,lua_tostring(NL,-1));
		return;
	}

	if(sd && sd->script_state==NRUN) { // If the script has finished (not waiting answer from client)
	    sd->NL=NULL; // Close the player's personal thread
		sd->npc_id=0; // Set the player's current NPC to 'none'
	}
}

// Run a Lua chunk
void script_run_chunk(const char *chunk,int char_id)
{
	lua_State *NL;
    struct map_session_data *sd=NULL;

	if (char_id == 0) { // If char_id points to no player
		NL = L; // Use the global thread
	} else { // Else we want to run the chunk for a specific player
		sd = map_charid2sd(char_id);
		nullpo_retv(sd);
		if(sd->script_state!=NRUN) { // Check that the player is not currently running a script
			ShowError("Cannot run chunk %s for player %d : player is currently running a script\n",chunk,char_id);
			return;
		}
		NL = sd->NL = lua_newthread(L); // Else the player's personal thread
		lua_pushliteral(NL,"char_id"); // Push global key for char_id
		lua_pushnumber(NL,char_id); // Push value for char_id
		lua_rawset(NL,LUA_GLOBALSINDEX); // Tell Lua to set char_id as a global var
	}
    
	luaL_loadbuffer(NL,chunk,strlen(chunk),"chunk"); // Pass chunk to Lua
	if (lua_pcall(NL,0,0,0)!=0){ // Tell Lua to run the chunk
		ShowError("Cannot run chunk %s : %s\n",chunk,lua_tostring(NL,-1));
		return;
	}

	if(sd && sd->script_state==NRUN) { // If the script has finished (not waiting answer from client)
	    sd->NL=NULL; // Close the player's personal thread
		sd->npc_id=0; // Set the player's current NPC to 'none'
	}
}

// Resume an already paused script
void script_resume(struct map_session_data *sd,const char *format,...) {
	va_list arg;
	int n=0;

	nullpo_retv(sd);

	if(sd->script_state==NRUN) { // Check that the player is currently running a script
		ShowError("Cannot resume script for player %d : player is not running a script\n",sd->char_id);
		return;
	}
	sd->script_state=NRUN; // Set the script flag as 'not waiting for anything'

	va_start(arg,format); // Initialize the argument list
	while (*format) { // Pass arguments to Lua, according to the types defined by "format"
        switch (*format++) {
          case 'd': // d = Double
            lua_pushnumber(sd->NL,va_arg(arg,double));
            break;
          case 'i': // i = Integer
            lua_pushnumber(sd->NL,va_arg(arg,int));
            break;
          case 's': // s = String
            lua_pushstring(sd->NL,va_arg(arg,char*));
            break;
          default: // Unknown code
            ShowError("%c : Invalid argument type code, allowed codes are 'd'/'i'/'s'\n",*(format-1));
        }
        n++;
        luaL_checkstack(sd->NL,1,"Too many arguments");
    }

    va_end(arg);

	if (lua_resume(sd->NL,n)!=0) { // Tell Lua to run the function
		ShowError("Cannot resume script for player %d : %s\n",sd->char_id,lua_tostring(sd->NL,-1));
		return;
	}

	if(sd->script_state==NRUN) { // If the script has finished (not waiting answer from client)
	    sd->NL=NULL; // Close the player's personal thread
		sd->npc_id=0; // Set the player's current NPC to 'none'
	}
}

static int set_posword(char *p)
{
	char* np,* str[15];
	int i=0;
	for(i=0;i<11;i++) {
		if((np=strchr(p,','))!=NULL) {
			str[i]=p;
			*np=0;
			p=np+1;
		} else {
			str[i]=p;
			p+=strlen(p);
		}
		if(str[i])
			strcpy(pos[i],str[i]);
	}
	return 0;
}

int script_config_read(char *cfgName)
{
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	script_config.verbose_mode = 0;
	script_config.warn_func_no_comma = 1;
	script_config.warn_cmd_no_comma = 1;
	script_config.warn_func_mismatch_paramnum = 1;
	script_config.warn_cmd_mismatch_paramnum = 1;
	script_config.check_cmdcount = 65535;
	script_config.check_gotocount = 2048;

	script_config.die_event_name = (char *) aCallocA (24, sizeof(char));
	script_config.kill_event_name = (char *) aCallocA (24, sizeof(char));
	script_config.login_event_name = (char *) aCallocA (24, sizeof(char));
	script_config.logout_event_name = (char *) aCallocA (24, sizeof(char));
	script_config.mapload_event_name = (char *) aCallocA (24, sizeof(char));

	fp = fopen(cfgName, "r");
	if (fp == NULL) {
		printf("file not found: %s\n", cfgName);
		return 1;
	}
	while (fgets(line, sizeof(line) - 1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		i = sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if (i != 2)
			continue;
		if(strcmpi(w1,"refine_posword")==0) {
			set_posword(w2);
		}
		else if(strcmpi(w1,"verbose_mode")==0) {
			script_config.verbose_mode = battle_config_switch(w2);
		}
		else if(strcmpi(w1,"warn_func_no_comma")==0) {
			script_config.warn_func_no_comma = battle_config_switch(w2);
		}
		else if(strcmpi(w1,"warn_cmd_no_comma")==0) {
			script_config.warn_cmd_no_comma = battle_config_switch(w2);
		}
		else if(strcmpi(w1,"warn_func_mismatch_paramnum")==0) {
			script_config.warn_func_mismatch_paramnum = battle_config_switch(w2);
		}
		else if(strcmpi(w1,"warn_cmd_mismatch_paramnum")==0) {
			script_config.warn_cmd_mismatch_paramnum = battle_config_switch(w2);
		}
		else if(strcmpi(w1,"check_cmdcount")==0) {
			script_config.check_cmdcount = battle_config_switch(w2);
		}
		else if(strcmpi(w1,"check_gotocount")==0) {
			script_config.check_gotocount = battle_config_switch(w2);
		}
		else if(strcmpi(w1,"die_event_name")==0) {			
			strcpy(script_config.die_event_name, w2);			
		}
		else if(strcmpi(w1,"kill_event_name")==0) {
			strcpy(script_config.kill_event_name, w2);
		}
		else if(strcmpi(w1,"login_event_name")==0) {
			strcpy(script_config.login_event_name, w2);
		}
		else if(strcmpi(w1,"logout_event_name")==0) {
			strcpy(script_config.logout_event_name, w2);
		}
		else if(strcmpi(w1,"mapload_event_name")==0) {
			strcpy(script_config.mapload_event_name, w2);
		}
		else if(strcmpi(w1,"import")==0){
			script_config_read(w2);
		}
	}
	fclose(fp);

	return 0;
}

/*==========================================
 * èIóπ
 *------------------------------------------
 */
/*static int mapreg_db_final(void *key,void *data,va_list ap)
{
	return 0;
}
static int mapregstr_db_final(void *key,void *data,va_list ap)
{
	aFree(data);
	return 0;
}*/

int do_final_script()
{
/*	if(mapreg_dirty>=0)
		script_save_mapreg();

	if(mapreg_db)
		numdb_final(mapreg_db,mapreg_db_final);
	if(mapregstr_db)
		strdb_final(mapregstr_db,mapregstr_db_final);*/

	if (script_config.die_event_name)
		aFree(script_config.die_event_name);
	if (script_config.kill_event_name)
		aFree(script_config.kill_event_name);
	if (script_config.login_event_name)
		aFree(script_config.login_event_name);
	if (script_config.logout_event_name)
		aFree(script_config.logout_event_name);
	if (script_config.mapload_event_name)
		aFree(script_config.mapload_event_name);

	lua_close(L); // Close Lua

	return 0;
}
/*==========================================
 * èâä˙âª
 *------------------------------------------
 */
int do_init_script()
{
/*	mapreg_db=numdb_init();
	mapregstr_db=numdb_init();
	script_load_mapreg();

	add_timer_func_list(script_autosave_mapreg,"script_autosave_mapreg");
	add_timer_interval(gettick()+MAPREG_AUTOSAVE_INTERVAL,
		script_autosave_mapreg,0,0,MAPREG_AUTOSAVE_INTERVAL);*/

	L = lua_open(); // Open Lua
	luaopen_base(L); // Open the basic library
	luaopen_table(L); // Open the table library
	luaopen_io(L); // Open the I/O library
	luaopen_string(L); // Open the string library
	luaopen_math(L); // Open the math library

	lua_pushliteral(L,"char_id"); // Push global key for char_id
	lua_pushnumber(L,0); // Push value 0 for char_id (the global thread is not linked to any player !)
	lua_rawset(L,LUA_GLOBALSINDEX); // Tell Lua to set char_id as a global var

	script_buildin_commands(); // Build in the Lua commands

	return 0;
}
