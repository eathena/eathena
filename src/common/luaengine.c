// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/showmsg.h"
#include "../common/malloc.h"
#include "../common/utils.h"

#include "luaengine.h"

#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>


/// Global Lua state.
static lua_State* g_L = NULL;



/// Allocator function for the lua state.
static void* luaengine_allocator(void* ud, void* ptr, size_t osize, size_t nsize)
{
	(void)ud; (void)osize;  // not used

	if( nsize == 0 )
	{
		aFree(ptr);
		return NULL;
	}

	return aRealloc(ptr, nsize);
}


//---------------------------------------------
// PUBLIC
//


/// Return the global state.
lua_State* luaengine_state(void)
{
	return g_L;
}


/// Create the global Lua state.
/// @param openmorelibs Optional function that opens aditional libraries
void do_init_luaengine(lua_CFunction openmorelibs)
{
	lua_State* L = NULL;
	const char* str = NULL;
	char subdir, nextpath, target, exedir;

	g_L = L = lua_newstate(luaengine_allocator, NULL);

	// standard libraries
	luaL_openlibs(L);

	// only accept packages in the lua dir
	lua_getglobal(L, "package");
	// package config
	lua_getfield(L, -1, "config");
	str = lua_tostring(L, -1);
	subdir = str[0];
	nextpath = str[2];
	target = str[4];
	exedir = str[6];
	lua_pop(L, 1);
	// lua packages
	lua_pushfstring(L,
		"%c%c%s%c%c%s" "%c"
		"%c%c%s%c%c%c%s",
		exedir, subdir, "lua", subdir, target, ".lua", nextpath,
		exedir, subdir, "lua", subdir, target, subdir, "init.lua");
	lua_setfield(L, -2, "path");
	// dll packages
	lua_pushfstring(L,
		"%c%c%s%c%c%s" "%c"
		"%c%c%s%c%c%s" "%c"
		"%c%c%s%c%s" "%c"
		"%c%c%s%c%s",
		exedir, subdir, "lua", subdir, target, ".dll", nextpath,
		exedir, subdir, "lua", subdir, target, ".so", nextpath,
		exedir, subdir, "lua", subdir, "loadall.dll", nextpath,
		exedir, subdir, "lua", subdir, "loadall.so");
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	// other libraries
	if( openmorelibs )
		openmorelibs(L);

	// OPTIONAL setup environment with init.lua
	str = "."LUA_DIRSEP"lua"LUA_DIRSEP"init.lua";
	if( exists(str) && luaL_dofile(L, str) )
	{
		if( lua_gettop(L) > 0 )
			ShowFatalError("do_init_luaengine: %s\n", lua_tostring(L, -1));
		else
			ShowFatalError("do_init_luaengine: unknown error in %s\n", str);
		exit(EXIT_FAILURE);
	}
	lua_settop(L, 0);
}


///  Destroy the global Lua state.
void do_final_luaengine()
{
	lua_close(g_L);
	g_L = NULL;
}
