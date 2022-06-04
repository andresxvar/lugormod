extern "C"
{
#include "lua/lua.h"
#include "lua/lauxlib.h"
}
#include "g_local.h"
#include "g_lua_main.h"

//
// Game.BindCommand(name:String, command:Function)
//
static int g_lua_Game_BindCommand(lua_State *L)
{
	int n = lua_gettop(L), cmdref = LUA_REFNIL;
	int i;
	char *name = NULL;

	if (n < 2)
		return luaL_error(L, "syntax: BindCommand(name:String, command:Function)");

	// get the first argument, must be a string
	name = (char *)luaL_checkstring(L, 1);

	// check if the first argument is a string
	luaL_argcheck(L, name != NULL, 1, "`string' expected");

	// check if the second argument is a function or a nil value
	if (!lua_isfunction(L, 2) && !lua_isnil(L, 2))
		return luaL_error(L, "second argument is not a function or a nil value");

	for (i = 0; i < MAX_LUA_CMDS; i++)
	{
		if (st_lua_cmds[i].function && !Q_stricmp(st_lua_cmds[i].name, name) && lua_isnil(L, 2))
		{
			luaL_unref(L, LUA_REGISTRYINDEX, st_lua_cmds[i].function);
			memset(&st_lua_cmds[i], 0, sizeof(st_lua_cmd_t));
			st_lua_cmds[i].name = 0;
			st_lua_cmds[i].function = 0;
			return 0;
		}
	}

	if (!lua_isfunction(L, 2))
		return luaL_error(L, "second argument must be a function");

	// push the value of the second argument and register it
	// inside the registry
	lua_pushvalue(L, 2);
	cmdref = luaL_ref(L, LUA_REGISTRYINDEX);

	for (i = 0; i < MAX_LUA_CMDS; i++)
	{
		if (!st_lua_cmds[i].function)
		{
			st_lua_cmds[i].name = G_NewString(name);
			st_lua_cmds[i].function = cmdref;
			return 0;
		}
	}
	return 0;
}

//
// Game.Broadcast( message:String )
// Game.Broadcast( message:String, flags:Number )
// Game.Broadcast( message:String, flags:Number, playerId:Number )
//
static int g_lua_Game_Broadcast(lua_State *L)
{
	int n = lua_gettop(L);
	char *message = NULL;
	int flags = 0;
	int playerId = -1;

	// get the message to print
	message = (char *)luaL_checkstring(L, 1);
	luaL_argcheck(L, message != NULL, 1, "`string' expected");

	if (n > 1)
		flags = luaL_checkinteger(L, 2);
	if (n > 2)
		playerId = luaL_checkinteger(L, 3);

	if (flags & 16) // print to chat
		trap_SendServerCommand(playerId, va("chat \"%s\"", message));
	else if (flags & 8) // print to console
		trap_SendServerCommand(playerId, va("print \"%s\"", message));
	else // center prints
		trap_SendServerCommand(playerId, va("cp \"%s\"", message));

	return 0;
}

//
// Game.PlayEffect( effect:String, origin:Vector)
//
static int g_lua_Game_PlayEffect(lua_State *L)
{
	int n = lua_gettop(L);
	char *effect = NULL;
	vec_t *origin;
	vec3_t up = {-90, 0, 0}; // play effects up
	if (n < 2)
		return luaL_error(L, "syntax: PlayEffect( effect:String, origin:Vector )");

	effect = (char *)luaL_checkstring(L, 1);

	origin = lua_getvector(L, 2);
	luaL_argcheck(L, origin != NULL, 2, "`QVector' expected");

	G_PlayEffectID(G_EffectIndex(effect), origin, up);
	return 0;
}

//
// Game.ConcatArgs(index:Integer)
//
static int g_lua_Game_ConcatArgs(lua_State *L)
{
	int n;

	n = luaL_optinteger(L, 1, 1);
	lua_pushstring(L, ConcatArgs(n));
	return 1;
}

//
// Game.Argument(index:Number)
//
static int g_lua_Game_Argument(lua_State *L)
{
	int argn;
	char result[64];

	argn = luaL_optinteger(L, 1, 0);
	trap_Argv(argn, result, sizeof(result));

	lua_pushstring(L, result);
	return 1;
}

//
// Game.Time()
//
static int g_lua_Game_Time(lua_State * L)
{
	lua_pushinteger(L, level.time);
	return 1;
}

//
// Game.PlaySound()
//
static int g_lua_Game_PlaySound(lua_State *L)
{
	return 1;
}

// game library methods
static const luaL_Reg GameRegistry[] = {
	{"BindCommand", g_lua_Game_BindCommand},
	{"Broadcast", g_lua_Game_Broadcast},
	{"Argument", g_lua_Game_Argument},
	{"ConcatArgs", g_lua_Game_ConcatArgs},
	{"PlayEffect", g_lua_Game_PlayEffect},
	{"PlaySound", g_lua_Game_PlaySound},
	{"Time", g_lua_Game_Time },

	{NULL, NULL}};

int luaopen_game(lua_State *L)
{
	luaL_newlib(L, GameRegistry);
	lua_setglobal(L, "Game");

	lua_pushliteral(L, "GAMEVERSION");
	lua_pushliteral(L, "lugormod");
	return 1;
}