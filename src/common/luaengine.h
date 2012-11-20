// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LUAENGINE_H_
#define _LUAENGINE_H_

#include <lua.h>

lua_State* luaengine_state(void);

void do_init_luaengine(lua_CFunction openmorelibs);
void do_final_luaengine(void);

#endif /* _LUAENGINE_H_ */
