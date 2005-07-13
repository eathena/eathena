/* LUA_EA module ***Kevin*** lua_ea.c v1.0 */

#include "base.h"
#include "socket.h"
#include "timer.h"
#include "malloc.h"
#include "lock.h"
#include "nullpo.h"
#include "db.h"
#include "socket.h"
#include "showmsg.h"
#include "utils.h"
#include "log.h"


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
#include "charcommand.h"
#include "lua_ea.h"

struct lua_ea_src_list *lua_ea_src_first=NULL;
struct lua_ea_src_list *lua_ea_src_last=NULL;

lua_State *lua_ea_L;
	
/***********************************************************\
|              LUA BUILDIN COMMANDS GO HERE                 |
\***********************************************************/

// scriptend()
// Exits definitively the script
static int buildin_scriptend(lua_State *NL)
{
	
	struct map_session_data *sd = lua_ea_get_target(NL, 1);
	lua_yield(NL, 0);
	lua_ea_close_script(sd);
	return 0;
}

// addnpc("NPC name","name postfix","map.gat",x,y,dir,sprite,"function")
// Add an standard NPC that triggers a function when clicked
static int buildin_addnpc(lua_State *NL)
{
	char *name,*exname,*map,*function;
	int m,x,y,dir,class_;
	
	name=(char *)aCallocA(strlen((char *)lua_tostring(NL, 1)), sizeof(char));
	name[0]=0;
	exname=(char *)aCallocA(strlen((char *)lua_tostring(NL, 2)), sizeof(char));
	exname[0]=0;
	map=(char *)aCallocA(strlen((char *)lua_tostring(NL, 3)), sizeof(char));
	map[0]=0;
	function=(char *)aCallocA(strlen((char *)lua_tostring(NL, 8)), sizeof(char));
	function[0]=0;
	
	strcpy(name,(char *)lua_tostring(NL,1));
	strcpy(exname,(char *)lua_tostring(NL,2));
	strcpy(map,(char *)lua_tostring(NL,3));
	m=map_mapname2mapid(map);
	x=(int)lua_tonumber(NL,4);
	y=(int)lua_tonumber(NL,5);
	dir=(int)lua_tonumber(NL,6);
	class_=(int)lua_tonumber(NL,7);
	strcpy(function,(char *)lua_tostring(NL,8));
	
	lua_ea_npc_add(name,exname,m,x,y,dir,class_,function);
	
	aFree(name);
	aFree(exname);
	aFree(map);
	aFree(function);

	return 0;
}

// addareascript("Area script name","map.gat",x1,y1,x2,y2,"function")
// Add an invisible area that triggers a function when entered
static int buildin_addareascript(lua_State *NL)
{
	char *name,*map,*function;
	int m,x1,y1,x2,y2;
	
	name=(char *)aCallocA(strlen((char *)lua_tostring(NL, 1)), sizeof(char));
	name[0]=0;
	map=(char *)aCallocA(strlen((char *)lua_tostring(NL, 2)), sizeof(char));
	map[0]=0;
	function=(char *)aCallocA(strlen((char *)lua_tostring(NL, 7)), sizeof(char));
	function[0]=0;

	strcpy(name,(char *)lua_tostring(NL,1));
	strcpy(map,(char *)lua_tostring(NL,2));
	m=map_mapname2mapid(map);
	x1=((int)lua_tonumber(NL,5)>(int)lua_tonumber(NL,3))?(int)lua_tonumber(NL,3):(int)lua_tonumber(NL,5);
	y1=((int)lua_tonumber(NL,6)>(int)lua_tonumber(NL,4))?(int)lua_tonumber(NL,4):(int)lua_tonumber(NL,6);
	x2=((int)lua_tonumber(NL,5)>(int)lua_tonumber(NL,3))?(int)lua_tonumber(NL,5):(int)lua_tonumber(NL,3);
	y2=((int)lua_tonumber(NL,6)>(int)lua_tonumber(NL,4))?(int)lua_tonumber(NL,6):(int)lua_tonumber(NL,4);
	strcpy(function,(char *)lua_tostring(NL,7));

	lua_ea_areascript_add(name,m,x1,y1,x2,y2,function);
	
	aFree(name);
	aFree(map);
	aFree(function);

	return 0;
}

// addwarp("Warp name","map.gat",x,y,"destmap.gat",destx,desty,xradius,yradius)
// Add a warp that moves players to somewhere else when entered

/*static int buildin_addwarp(lua_State *NL) // not sure about warps yet with eapp, will look into it
{
	char name[24],map[16],destmap[16];
	short m,x,y;
	short destx,desty,xs,ys;
	
	sprintf(name,"%s",lua_tostring(NL,1));
	sprintf(map,"%s",lua_tostring(NL,2));
	m=map_mapname2mapid(map);
	x=(int)lua_tonumber(NL,3);
	y=(int)lua_tonumber(NL,4);
	sprintf(destmap,"%s",lua_tostring(NL,5));
	destx=(int)lua_tonumber(NL,6);
	desty=(int)lua_tonumber(NL,7);
	xs=(int)lua_tonumber(NL,8);
	ys=(int)lua_tonumber(NL,9);

	lua_ea_warp_add(name,m,x,y,destmap,destx,desty,xs,ys);

	return 0;
}

// addspawn("Monster name","map.gat",x,y,xradius,yradius,id,amount,delay1,delay2,"function")
// Add a monster spawn etc
static int buildin_addspawn(lua_State *NL) //or mob spawns
{
	char name[24],map[16],function[50];
	short m,x,y,xs,ys,class_,num,d1,d2;

	sprintf(name,"%s",lua_tostring(NL,1));
	sprintf(map,"%s",lua_tostring(NL,2));
	m=map_mapname2mapid(map);
	x=(int)lua_tonumber(NL,3);
	y=(int)lua_tonumber(NL,4);
	xs=(int)lua_tonumber(NL,5);
	ys=(int)lua_tonumber(NL,6);
	class_=(int)lua_tonumber(NL,7);
	num=(int)lua_tonumber(NL,8);
	d1=(int)lua_tonumber(NL,9);
	d2=(int)lua_tonumber(NL,10);
	sprintf(function,"%s",lua_tostring(NL,11));

	lua_ea_spawn_add(name,m,x,y,xs,ys,class_,num,d1,d2,function);

	return 0;
}*/

// npcmes("Text",[id])
// Print the text into the NPC dialog window of the player
static int buildin_npcmes(lua_State *NL)
{
	struct map_session_data *sd;
	char mes[512];

	sprintf(mes,"%s",lua_tostring(NL, 1)); 
	sd = lua_ea_get_target(NL, 2);

	clif_scriptmes(*sd, sd->lua_player_data.script_id, mes);

	return 0;
}

// npcclose([id])
// Display a [Close] button in the NPC dialog window of the player and pause the script until the button is clicked
static int buildin_npcclose(lua_State *NL)
{
	struct map_session_data *sd;

	sd = lua_ea_get_target(NL, 1);

	clif_scriptclose(*sd,sd->lua_player_data.script_id);

	sd->lua_player_data.state = CLOSE;
	return lua_yield(NL, 0);
}

// npcnext([id])
// Display a [Next] button in the NPC dialog window of the player and pause the script until the button is clicked
static int buildin_npcnext(lua_State *NL)
{
	struct map_session_data *sd;

	sd = lua_ea_get_target(NL, 1);

	clif_scriptnext(*sd,sd->lua_player_data.script_id);

	sd->lua_player_data.state = NEXT;
	return lua_yield(NL, 0);
}

// npcinput(type,[id])
// Display an NPC input window asking the player for a value
static int buildin_npcinput(lua_State *NL)
{
	struct map_session_data *sd;
	int type;

	type = (int)lua_tonumber(NL, 1);
	sd = lua_ea_get_target(NL, 2);

	switch(type){
		case 0:
			clif_scriptinput(*sd,sd->lua_player_data.script_id);
			sd->lua_player_data.state = INPUT;
			break;
		case 1:
			clif_scriptinputstr(*sd,sd->lua_player_data.script_id);
			sd->lua_player_data.state = INPUT_STR;
			break;
	}

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
	int value, index;

	sd = lua_ea_get_target(NL, 2);
	
	//setup table to store values
	lua_pushliteral(NL, "menu");
	lua_newtable(NL);
	lua_rawset(NL, LUA_GLOBALSINDEX);

	lua_pushliteral(NL, "n");
	lua_rawget(NL, 1);
	n = (int)lua_tonumber(NL, -1);
	lua_pop(NL, 1);

	if(n%2 == 1) {
		lua_pushstring(NL, "Incorrect number of arguments for function 'npcmenu'\n");
		lua_error(NL);
		return -1;
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

	for(i=0, index=0; i<n; i+=2, index++) {
		lua_pushnumber(NL, i+1);
		lua_rawget(NL, 1);
		menu = (char *)lua_tostring(NL, -1);
		lua_pop(NL, 1);

		lua_pushnumber(NL, i+2);
		lua_rawget(NL, 1);
		value = (int)lua_tonumber(NL, -1);
		lua_pop(NL, 1);
		
		lua_pushliteral(NL, "menu");
		lua_rawget(NL, LUA_GLOBALSINDEX);
		lua_pushnumber(NL, index);
		lua_pushnumber(NL, value);
		lua_rawset(NL, -3);
		lua_pop(NL, 1);
		
		strcat(buf,menu);
		strcat(buf,":");
	}

	clif_scriptmenu(*sd,sd->lua_player_data.script_id,buf);
	aFree(buf);

	sd->lua_player_data.state = MENU;
	return lua_yield(NL, 1);
}

// npcshop(item_id1,item_price1,...)
// Start a shop with buylist of item_id selling for item_price
static int buildin_npcshop(lua_State *NL)
{
	struct map_session_data *sd;
	int n, i, j;
	int id, value;
	
	//Set up shop data table
	lua_pushliteral(NL, "shop");
	lua_newtable(NL);
	lua_rawset(NL, LUA_GLOBALSINDEX);

	sd = lua_ea_get_target(NL, 2);

	lua_pushliteral(NL, "n");
	lua_rawget(NL, 1);
	n = (int)lua_tonumber(NL, -1);
	lua_pop(NL, 1);

	if(n%2 == 1) {
		lua_pushstring(NL, "Incorrect number of arguments for function 'npcshop'\n");
		lua_error(NL);
		return -1;
	}

	lua_pushliteral(NL, "shop");
	lua_rawget(NL, LUA_GLOBALSINDEX);
	lua_pushliteral(NL, "n");
	lua_pushnumber(NL, n/2);
	lua_rawset(NL, -3);
	lua_pop(NL, 1);

	for(i=1; i<=n; i+=2) {
		lua_pushnumber(NL, i);
		lua_rawget(NL, 1);
		id = (int)lua_tonumber(NL, -1);
		lua_pop(NL, 1);
		lua_pushnumber(NL, i+1);
		lua_rawget(NL, 1);
		value = (int)lua_tonumber(NL, -1);
		lua_pop(NL, 1);
		
		lua_pushliteral(NL, "shop");
		lua_rawget(NL, LUA_GLOBALSINDEX);
		lua_pushnumber(NL, i);
		lua_pushnumber(NL, id);
		lua_rawset(NL, -3);
		lua_pushnumber(NL, i+1);
		lua_pushnumber(NL, value);
		lua_rawset(NL, -3);
		lua_pop(NL, 1);
	}

	clif_npcbuysell(*sd, sd->lua_player_data.script_id);

	sd->lua_player_data.state = LUA_SHOP;
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
	type = (int)lua_tonumber(NL, 2);
	sd = lua_ea_get_target(NL, 3);

	clif_cutin(*sd,name,type);

	return 0;
}

// heal(hp,sp,[id])
// Heal the character by a set amount of HP and SP
static int buildin_heal(lua_State *NL)
{
	struct map_session_data *sd;
	int hp, sp;

	hp = (int)lua_tonumber(NL, 1);
	sp = (int)lua_tonumber(NL, 2);
	sd = lua_ea_get_target(NL, 3);

	pc_heal(*sd, hp, sp);

	return 0;
}

// percentheal(hp,sp,[id])
// Heal the character by a percentage of MaxHP and MaxSP
static int buildin_percentheal(lua_State *NL)
{
	struct map_session_data *sd;
	int hp, sp;

	hp = (int)lua_tonumber(NL, 1);
	sp = (int)lua_tonumber(NL, 2);
	sd = lua_ea_get_target(NL, 3);

	pc_percentheal(*sd, hp, sp);

	return 0;
}

// itemheal(hp,sp,[id])
// Heal the character by an amount of HP and SP that increases with VIT/INT, skills, etc
static int buildin_itemheal(lua_State *NL)
{
	struct map_session_data *sd;
	int hp, sp;

	hp = (int)lua_tonumber(NL, 1);
	sp = (int)lua_tonumber(NL, 2);
	sd = lua_ea_get_target(NL, 3);

	pc_itemheal(*sd, hp, sp);

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
	x = (int)lua_tonumber(NL, 2);
	y = (int)lua_tonumber(NL, 3);
	sd = lua_ea_get_target(NL, 4);

	if(strcmp(str,"Random")==0) // Warp to random location
		pc_randomwarp(*sd,3);
	else if(strcmp(str,"SavePoint")==0 || strcmp(str,"Save")==0) { // Warp to save point
		if(map[sd->bl.m].flag.noreturn)
			return 0;
		pc_setpos(*sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);
	} else // Warp to given location
		pc_setpos(*sd,str,x,y,0);

	return 0;
}

// jobchange(job,[id])
// Change the job of the character
static int buildin_jobchange(lua_State *NL)
{
	struct map_session_data *sd;
	int job;

	job = (int)lua_tonumber(NL, 1);
	sd = lua_ea_get_target(NL, 2);

	pc_jobchange(*sd,job,0);

	return 0;
}

// setlook(type,val,[id])
// Change the look of the character
static int buildin_setlook(lua_State *NL)
{
	struct map_session_data *sd;
	int type,val;

	type = (int)lua_tonumber(NL, 1);
	val = (int)lua_tonumber(NL, 2);
	sd = lua_ea_get_target(NL, 3);

	pc_changelook(*sd,type,val);

	return 0;
}

/*****************************************************************\
|       END LUA BUILDINGS/PUT THEM BELOW TO REGISTER THEM         |
\*****************************************************************/

// List of commands to build into Lua, format : {"function_name_in_lua", C_function_name}
static struct LuaCommandInfo commands[] = {
	
	// Basic functions
	{"scriptend", buildin_scriptend},
	
	// Object creation functions
	{"addnpc", buildin_addnpc},
	{"addareascript", buildin_addareascript},
//	{"addwarp", buildin_addwarp},
//	{"addspawn", buildin_addspawn},
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

/***********************************************************\
|                BEGIN LUA SYSTEM FUNTIONS                  |
\***********************************************************/

int lua_ea_get_new_npc_id(void)
{
	return npc_id++;
}

// Register build-in commands specified above
void lua_ea_buildin_commands()
{
	int i=0;

	while(commands[i].f) {
		lua_pushstring(lua_ea_L, commands[i].command);
        lua_pushcfunction(lua_ea_L, commands[i].f);
        lua_settable(lua_ea_L, LUA_GLOBALSINDEX);
        i++;
    }
	ShowStatus("Done registering '"CL_BLUE"%d"CL_RESET"' script build-in commands.\n",i);
}

void lua_ea_load_script_files()
{
	char *name;
	struct lua_ea_src_list *sls;
	
	for(sls=lua_ea_src_first;sls;sls=sls->next) {
		name = sls->file;
		
		FILE *fp = fopen (name,"r");
		if (fp == NULL) {
			ShowError("File not found : %s\n",name);
			exit(1);
		}
	
		if (luaL_loadfile(lua_ea_L,name))
			ShowError("Cannot load script file %s : %s",name,lua_tostring(lua_ea_L,-1));
		if (lua_pcall(lua_ea_L,0,0,0))
			ShowError("Cannot run script file %s : %s",name,lua_tostring(lua_ea_L,-1));

		fclose(fp);
	}
}
	

// Run a Lua function that was previously loaded, specifying the type of arguments with a "format" string
void lua_ea_run_function(const char *name,int char_id,const char *format,...)
{
	va_list arg;
	lua_State *NL;
	struct map_session_data *sd;
	int n=0;

	if (char_id == 0) { // If char_id points to no player
		NL = lua_ea_L; // Use the global thread
	} else { // Else we want to run the function for a specific player
		sd = map_charid2sd(char_id);
		nullpo_retv(sd);
		if(sd->lua_player_data.state!=NRUN) { // Check that the player is not currently running a script
			ShowError("Cannot run lua function %s for player %d : script state is not dead\n",name,char_id);
			return;
		}
		NL = sd->lua_player_data.NL = lua_newthread(lua_ea_L); // Use the player's personal thread
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
            ShowError("%c : Invalid lua argument type code, allowed codes are 'd'/'i'/'s'\n",*(format-1));
        }
        n++;
        luaL_checkstack(NL,1,"Too lua many arguments!\n");
    }

	va_end(arg);

	if (lua_resume(NL,n)!=0) { // Tell Lua to run the function
		ShowError("Cannot run lua function %s : %s\n",name,lua_tostring(NL,-1));
		return;
	}
}

// Run a Lua chunk
void lua_ea_run_chunk(const char *chunk,int char_id)
{
	lua_State *NL;
    struct map_session_data *sd=NULL;

	if (char_id == 0) { // If char_id points to no player
		NL = lua_ea_L; // Use the global thread
	} else { // Else we want to run the chunk for a specific player
		sd = map_charid2sd(char_id);
		nullpo_retv(sd);
		if(sd->lua_player_data.state!=NRUN) { // Check that the player is not currently running a script
			ShowError("Cannot run lua chunk %s for player %d : script is state not dead\n",chunk,char_id);
			return;
		}
		NL = sd->lua_player_data.NL = lua_newthread(lua_ea_L); // Else the player's personal thread
		lua_pushliteral(NL,"char_id"); // Push global key for char_id
		lua_pushnumber(NL,char_id); // Push value for char_id
		lua_rawset(NL,LUA_GLOBALSINDEX); // Tell Lua to set char_id as a global var
	}
    
	luaL_loadbuffer(NL,chunk,strlen(chunk),"chunk"); // Pass chunk to Lua
	if (lua_pcall(NL,0,0,0)!=0){ // Tell Lua to run the chunk
		ShowError("Cannot run lua chunk %s : %s\n",chunk,lua_tostring(NL,-1));
		return;
	}
}

// Resume an already paused script
void lua_ea_resume(struct map_session_data *sd,const char *format,...) {
	va_list arg;
	int n=0;

	nullpo_retv(sd);

	if(sd->lua_player_data.state==NRUN) { // Check that the player is currently running a script
		ShowError("Cannot resume lua script for player %d : script state is dead\n",sd->status.char_id);
		return;
	}

	va_start(arg,format); // Initialize the argument list
	while (*format) { // Pass arguments to Lua, according to the types defined by "format"
        switch (*format++) {
          case 'd': // d = Double
            lua_pushnumber(sd->lua_player_data.NL,va_arg(arg,double));
            break;
          case 'i': // i = Integer
            lua_pushnumber(sd->lua_player_data.NL,va_arg(arg,int));
            break;
          case 's': // s = String
            lua_pushstring(sd->lua_player_data.NL,va_arg(arg,char*));
            break;
          default: // Unknown code
            ShowError("%c : Invalid argument type code, allowed codes are 'd'/'i'/'s'\n",*(format-1));
        }
        n++;
        luaL_checkstack(sd->lua_player_data.NL,1,"Too many lua arguments!\n");
    }

    va_end(arg);

	if (lua_resume(sd->lua_player_data.NL,n)!=0) { // Tell Lua to run the function
		ShowError("Cannot resume lua script for player %d : %s\n",sd->status.char_id,lua_tostring(sd->lua_player_data.NL,-1));
		return;
	}
}

int lua_ea_npc_add(char *name,char *exname,int m,int x,int y,int dir,int class_,char *function)
{
	struct npc_data *nd=(struct npc_data *)aCalloc(1,sizeof(struct npc_data));

	nd->bl.prev=nd->bl.next=NULL;
	nd->bl.m=m;
	nd->bl.x=x;
	nd->bl.y=y;
	nd->bl.id=lua_ea_get_new_npc_id();
	nd->bl.type=BL_NPC;
	nd->bl.subtype=LUA;
	strcpy(nd->name,name);
	strcpy(nd->exname,exname);
	strcpy(nd->lua_npc_data.function,function);
	nd->dir=dir;
	nd->class_=class_;
	nd->flag=0;
	nd->option=0;
	nd->opt1=0;
	nd->opt2=0;
	nd->opt3=0;
	nd->chat_id=0;
	
	nd->lua_npc_data.type=NPC;

	nd->n=map_addnpc(m,nd);
	map_addblock(nd->bl);
	clif_spawnnpc(*nd);
	//strdb_insert(npcname_db,nd->exname,nd);
	npc_id++;

	return 0;
}

int lua_ea_areascript_add(char *name,int m,int x1,int y1,int x2,int y2,char *function)
{
	struct npc_data *nd=(struct npc_data *)aCalloc(1,sizeof(struct npc_data));

	nd->bl.prev=nd->bl.next=NULL;
	nd->bl.m=m;
	nd->bl.x=x1;
	nd->bl.y=y1;
	nd->bl.id=lua_ea_get_new_npc_id();
	nd->bl.type=BL_NPC;
	nd->bl.subtype=LUA;
	strcpy(nd->name,name);
	strcpy(nd->exname,name);
	nd->dir=0;
	nd->class_=0;
	nd->flag=0;
	nd->option=0;
	nd->opt1=0;
	nd->opt2=0;
	nd->opt3=0;
	nd->chat_id=0;
	
	//Specific lua data
	nd->lua_npc_data.type=AREASCRIPT;
	strcpy(nd->lua_npc_data.function,function);
	nd->lua_npc_data.ad.x1=x1;
	nd->lua_npc_data.ad.y1=y1;
	nd->lua_npc_data.ad.x2=x2;
	nd->lua_npc_data.ad.y2=y2;

	nd->n=map_addnpc(m,nd);
	map_addblock(nd->bl);
	clif_spawnnpc(*nd);
	//strdb_insert(npcname_db,nd->exname,nd);
	npc_id++;

	return 0;
}

// Check whether a char ID was passed as argument, else check if there's a global one
struct map_session_data * lua_ea_get_target(lua_State *NL,int idx)
{
	int char_id;
	struct map_session_data *sd;

	if((char_id=(int)lua_tonumber(NL, idx))==0) { // If 0 or nothing was passed as argument
		lua_pushliteral(NL, "char_id");
		lua_rawget(NL, LUA_GLOBALSINDEX);
		char_id=(int)lua_tonumber(NL, -1); // Get the thread's char ID if it's a personal one
		lua_pop(NL, 1);
	}

	if(char_id==0 || (sd=map_charid2sd(char_id))==NULL) { // If we still dont have a valid char ID here, there's a problem
		lua_pushstring(NL,"Character not found in lua script!");
		lua_error(NL);
		return NULL;
	}
	
	return sd;
}

//Send a buy list to the client
int lua_ea_sendbuylist(struct map_session_data *sd)
{
	struct item_data *id;
	int fd,i,j,val,n,nameid;
	lua_State *NL;

	nullpo_retr(0, sd);

	fd=sd->fd;
	NL = sd->lua_player_data.NL;
	
	lua_pushliteral(NL, "shop");
	lua_rawget(NL, LUA_GLOBALSINDEX);
	lua_pushliteral(NL, "n");
	lua_rawget(NL, -2);
	n = (int)lua_tonumber(NL, -1);
	lua_pop(NL, 1);
	
	WFIFOW(fd,0)=0xc6;
	for(i=0,j=1;i<n;i++,j+=2){
		
		lua_pushnumber(NL, j);
		lua_rawget(NL, -2);
		nameid = (int)lua_tonumber(NL, -1);
		lua_pop(NL, 1);
		lua_pushnumber(NL, j+1);
		lua_rawget(NL, -2);
		val = (int)lua_tonumber(NL, -1);
		lua_pop(NL, 2);
		
		id = itemdb_search(nameid);
		WFIFOL(fd,4+i*11)=val;
		if (!id->flag.value_notdc)
			val=pc_modifybuyvalue(*sd,val);
		WFIFOL(fd,8+i*11)=val;
		WFIFOB(fd,12+i*11)=id->type;
		if (id->view_id > 0)
			WFIFOW(fd,13+i*11)=id->view_id;
		else
			WFIFOW(fd,13+i*11)=nameid;
			
	}
	WFIFOW(fd,2)=i*11+4;
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

/***********************************************************\
|    END LUA SYSTEM FUNCTIONS/BEGIN LUA ACCESS FUNCTIONS    |
\***********************************************************/

/*** Initialization/Closing ***/

//Initialize lua
void init_lua_ea(void)
{
	lua_ea_L = lua_open(); // Open Lua
	luaopen_base(lua_ea_L); // Open the basic library
	luaopen_table(lua_ea_L); // Open the table library
	luaopen_io(lua_ea_L); // Open the I/O library
	luaopen_string(lua_ea_L); // Open the string library
	luaopen_math(lua_ea_L); // Open the math library

	lua_pushliteral(lua_ea_L,"char_id"); // Push global key for char_id
	lua_pushnumber(lua_ea_L,0); // Push value 0 for char_id (the global thread is not linked to any player !)
	lua_rawset(lua_ea_L,LUA_GLOBALSINDEX); // Tell Lua to set char_id as a global var

	lua_ea_buildin_commands(); // Build in the Lua commands
	
	lua_ea_script_file_add("script/core/core.lua");
	lua_ea_load_scripts(LUA_SCRIPTS_NAME);
	lua_ea_load_script_files();
	
}

//Close lua
void close_lua_ea(void)
{
	lua_close(lua_ea_L);
}

/*** Script Control ***/
//Run a lua script
int lua_ea_run(struct map_session_data *sd, struct npc_data *nd)
{
	
	if(sd->lua_player_data.state!=NRUN)
	{
		ShowError("Trying to start a non dead lua script!\n");
		return -1;
	}
	sd->lua_player_data.script_id = nd->bl.id;
	lua_ea_run_function(nd->lua_npc_data.function, sd->status.char_id, "");
	
	return 0;
	
}

//Continue a lua script
int lua_ea_continue(struct map_session_data *sd, char *format, ...)
{
	
	va_list args;
	int int_args[10];
	double double_args[10];
	char *char_args[10];
	int i=0, d=0, c=0, value;
	lua_State *NL=sd->lua_player_data.NL;
	
	va_start(args,format); // Initialize the argument list
	while (*format) {
        switch (*format++) {
          case 'd': // d = Double
            double_args[d] = va_arg(args,double);
            d++;
            break;
          case 'i': // i = Integer
            int_args[i] = va_arg(args,int);
            i++;
            break;
          case 's': // s = String
            char_args[c] = va_arg(args,char*);
            c++;
            break;
          default: // Unknown code
            ShowError("%c : Invalid lua argument type code, allowed codes are 'd'/'i'/'s'\n",*(format-1));
            return -1;
        }
        luaL_checkstack(NL,1,"Too many lua arguments!\n");
    }

	va_end(args);
	
	switch(sd->lua_player_data.state)
	{
	case NEXT: //Uses: lua_ea_continue(sd, nd, "");
	case CLOSE:
	case BUY:
	case SELL:
		lua_ea_resume(sd, "");
		break;
		
	case MENU: //Use: lua_ea_continue(sd, nd, "i", <value received from client>);
		lua_pushliteral(NL, "menu");
		lua_rawget(NL, LUA_GLOBALSINDEX);
		lua_pushnumber(NL,int_args[0]-1);
		lua_rawget(NL, -2);
		value = (int)lua_tonumber(NL, -1);
		lua_pop(NL, 2);
		
		lua_ea_resume(sd, "i", value);
		break;
		
	case INPUT: //Use: lua_ea_continue(sd, nd, "i", <value received from client>);
		lua_ea_resume(sd, "i", int_args[0]);
		break;
		
	case INPUT_STR: //Use: lua_ea_continue(sd, nd, "s", <value received from client>);
		lua_ea_resume(sd, "s", char_args[0]);
		break;
		
	case LUA_SHOP: //Use: lua_ea_continue(sd, nd, "i", <type>);
		if(!int_args[0])
		{
			sd->lua_player_data.state = BUY;
			lua_ea_sendbuylist(sd);
		}
		else
		{
			sd->lua_player_data.state = SELL;
			clif_selllist(*sd);
		}
		break;
		
	case NRUN: //Can't continue NRUN state
		ShowError("Lua script ran by Character ID '"CL_BLUE"%d"CL_RESET"' with npc id '"CL_YELLOW"%d"CL_RESET"' is not ready to be resumed!\n",
			sd->status.char_id, sd->lua_player_data.script_id);
		return -1;
		break;
		
	default:
		ShowError("Invalid lua resume state!\n");
		return -1;
		break;
			
	}
	
	return 0;
}

//close a script that a player is running
void lua_ea_close_script(struct map_session_data *sd)
{
	sd->lua_player_data.NL = NULL;
	sd->lua_player_data.state = NRUN;
	sd->lua_player_data.script_id = 0;
	sd->npc_id=0;
}

/************************************************************\
|                      SCRIPTS READING                       |
\************************************************************/

int lua_ea_load_scripts(const char *srcName)
{
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	fp=savefopen(srcName,"r");
	if (fp == NULL) {
		ShowMessage("file not found: %s\n",srcName);
		return 1;
	}
	while (fgets(line, sizeof(line) - 1, fp)) {
		if( !skip_empty_line(line) )
			continue;
		i = sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if (i != 2)
			continue;
		if(strcasecmp(w1,"lua")==0) {
			lua_ea_script_file_add(w2);
		}
		else if(strcasecmp(w1,"import")==0){
			lua_ea_load_scripts(w2);
		}
	}
	fclose(fp);

	return 0;
}

int lua_ea_script_file_add(char* file)
{
	struct lua_ea_src_list *sls;
	int len;
	
	sls = lua_ea_src_first;
	while(sls) //Don't load source file twice
	{
		if(!strcmp(file, sls->file)) //Found Match, don't add to list
			return 0;
		sls = sls->next; //Go to next node in list
	}
	
	len = sizeof(struct lua_ea_src_list) + strlen(file); // size of node + length of file name
	sls=(struct lua_ea_src_list *)aMalloc(len * sizeof(char)); // allocate proper memory to node
	
	sls->next = NULL;
	
	memcpy(sls->file, file, strlen(file)+1); //Copy file over to node
	if (lua_ea_src_first == NULL)
		lua_ea_src_first = sls; //Assign to first if first node
	if (lua_ea_src_last)//Assign to last's node next node if last node
		lua_ea_src_last->next = sls;
	lua_ea_src_last = sls; //Assign to last node
	
	return 0;
}
