#pragma once

extern "C"{
    #include "lua/lua.h" 
}
#include "Lmd_Entities_Public.h"

#define MAX_LUA_CMDS 128

// lua commands structure
typedef struct st_lua_cmd {
	char *name;
	int function;
} st_lua_cmd_t;

// lua entities
typedef struct st_lua_ent {
	char *name;
	int function;
	int logical;
} st_lua_ent_t;

extern st_lua_cmd_t st_lua_cmds[MAX_LUA_CMDS];

extern lua_State *g_lua;
extern int lua_toString;

void g_lua_init();
void g_lua_shutdown();
int g_lua_clientCommand(int clientId);
void g_lua_reportError();


// g_lua_game.c
int luaopen_game(lua_State * L);

// g_lua_entity.c
typedef struct
{
	gentity_t      *e;
} lua_GEntity;
extern int             luaopen_gentity(lua_State * L);
extern void            lua_pushgentity(lua_State * L, gentity_t * ent);
extern lua_GEntity    *lua_getgentity(lua_State * L, int argNum);
extern lua_GEntity 	  *lua_getcligentity(lua_State * L, int argNum);

// g_lua_vector.c
extern int		luaopen_vector(lua_State * L);
vec_t *lua_getvector(lua_State * L, int argNum);
void lua_pushvector(lua_State * L, vec_t *v);