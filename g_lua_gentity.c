extern "C"{
    #include "lua/lua.h"
    #include "lua/lauxlib.h"
}
#include "g_lua.h"

#include "g_local.h"

static int lua_GEntity_GC(lua_State * L)
{
	//G_Printf("Lua says bye to entity = %p\n", lua_getentity(L));
	return 0;
}

//
// GEntity.FromNumber( id:Integer )
//
static int lua_GEntity_FromNumber(lua_State *L)
{
	int n = lua_gettop(L), num;

	if (n < 1)
		return luaL_error(L, "syntax: GEntity.FromNumber( id:Integer )");

	num = luaL_checkinteger(L, 1);

	if (num > MAX_GENTITIES)
		return luaL_error(L, "number can't be more than %i", MAX_GENTITIES);

	lua_pushgentity(L, &g_entities[num]);
	return 1;
}

//
// GEntity:Number( )
//
static int lua_GEntity_Number(lua_State *L)
{
	lua_GEntity *lent;

	lent = lua_getgentity(L, 1);

	lua_pushinteger(L, lent->e->s.number);
	return 1;
}

//
// GEntity:cliName( )
// GEntity:cliName( newVal:String )
//
static int lua_GEntity_cliName(lua_State *L)
{
	int n = lua_gettop(L);
	lua_GEntity *lent = lua_getcligentity(L, 1);

	if (n > 1) {
		char *newVal = (char*)luaL_checkstring(L, 2);
		Q_strncpyz(lent->e->client->pers.netname, newVal, sizeof(lent->e->client->pers.netname));
		lent->e->client->pers.netnameTime = 0;
		ClientUserinfoChanged(lent->e->client->ps.clientNum);
		lent->e->client->pers.netnameTime = 0;
		return 0;
	}
	else
	{
		lua_pushstring(L, lent->e->client->pers.netname);
		return 1;
	}
}

static const luaL_Reg gentity_ctor[] = {
	{ "FromNumber", lua_GEntity_FromNumber },	
	
	{ NULL, NULL }
};

static const luaL_Reg gentity_meta[] = {
	{ "__gc", lua_GEntity_GC },

	{ "Number", lua_GEntity_Number },
	{ "cliName", lua_GEntity_cliName },

	{ NULL, NULL }
};

int luaopen_gentity(lua_State * L)
{
	// set GEntity object functions
	luaL_newlib(L, gentity_ctor);

	// set GEntity class metatable
	luaL_newmetatable(L, "Game.GEntity");  /* create metatable for entities */
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
	luaL_setfuncs(L, gentity_meta, 0);  /* add entity methods to new metatable */
	lua_pop(L, 1);  /* pop new metatable */

					// set global class
	lua_setglobal(L, "GEntity");

	return 1;
}

void lua_pushgentity(lua_State * L, gentity_t * ent)
{
	lua_GEntity     *lent;

	lent = (lua_GEntity*)lua_newuserdata(L, sizeof(lua_GEntity));

	luaL_getmetatable(L, "Game.GEntity");
	lua_setmetatable(L, -2);

	lent->e = ent;
}

lua_GEntity	*lua_getgentity(lua_State * L, int argNum)
{
	void *ud;
	lua_GEntity	*lent;

	ud = luaL_checkudata(L, argNum, "Game.GEntity");
	luaL_argcheck(L, ud != NULL, argNum, "`entity' expected");

	lent = (lua_GEntity *)ud;
	return lent;
}

lua_GEntity *lua_getcligentity(lua_State * L, int argNum)
{
	void *ud;
	lua_GEntity	*lent;

	ud = luaL_checkudata(L, argNum, "Game.GEntity");
	luaL_argcheck(L, ud != NULL, argNum, "`entity' expected");
	lent = (lua_GEntity *)ud;
	luaL_argcheck(L, lent->e->client != NULL, argNum, "entity is not a client");

	return lent;
}