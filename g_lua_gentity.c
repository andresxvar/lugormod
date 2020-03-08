extern "C" 
{
#include "lua/lua.h"
#include "lua/lauxlib.h"
}
#include "g_lua_main.h"

#include "g_local.h"
#include "Lmd_EntityCore.h"

static int g_lua_GEntity_GC(lua_State * L)
{
	//G_Printf("Lua says bye to entity = %p\n", lua_getentity(L));
	return 0;
}

//
// GEntity.FromNumber( id:Integer )
//
static int g_lua_GEntity_FromNumber(lua_State *L)
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
// GEntity.Place( Spawnstring:String )
// GEntity.Place( Spawnstring:String, CanSave:Boolean)
//
static int g_lua_GEntity_Place(lua_State *L)
{
	int n = lua_gettop(L);
	char *spawnString;
	gentity_t *result = NULL;
	SpawnData_t *spawnData = NULL;

	if (n < 1)
		return luaL_error(L, "syntax: Place( Spawnstring:String )");

	spawnString = (char*)luaL_checkstring(L, 1);
	result = trySpawn(spawnString);

	if (result)
	{
		if (n > 1)
		{
			int canSave = luaL_checkinteger(L, 2);
			if (canSave != 0)
				Lmd_Entities_SetSaveable(result->Lmd.spawnData, qtrue);				
		}

		lua_pushgentity(L, result);
	}
	else
		lua_pushnil(L);

	return 1;
}

//
// GEntity.Register( name:string, spawn:function, logical:bool)
//
// g_lua_Spawn 
// Handler for spawning custom lua entities
st_lua_ent_t st_lua_ents[MAX_LUA_ENTS];
void Lmd_AddSpawnableEntry(spawn_t spawnData);
void g_lua_Spawn(gentity_t *ent)
{	
	// pick the right spawning function
	for (int i = 0; i < MAX_LUA_ENTS; ++i)
	{
		if (!Q_stricmp(st_lua_ents[i].name, ent->classname))
		{
			lua_rawgeti(g_lua, LUA_REGISTRYINDEX, st_lua_ents[i].function);
			lua_pushgentity(g_lua, ent);
			if (lua_pcall(g_lua, 1, 0, 0))
				g_lua_reportError();
			return;
		}
	}
}
// g_lua_RegisterEntities
// Registers a new entity see GEntity.Register()
void g_lua_RegisterEntities()
{
	// register all lua entities
	for (int i = 0; i < MAX_LUA_ENTS; ++i)
	{
		if (st_lua_ents[i].function)
		{
			spawn_t data = { st_lua_ents[i].name,g_lua_Spawn, st_lua_ents[i].logical, NULL };
			Lmd_AddSpawnableEntry(data);
		}
		else
		{
			break;
		}
	}
}
static int g_lua_GEntity_Register(lua_State *L)
{
	int n = lua_gettop(L), spref = LUA_REFNIL, logical = 0;
	char *name;
	int i;

	if (n < 3)
		return luaL_error(L, "syntax: Register(name:String, spawn:Function, islogical:Boolean)");

	// get the first argument, must be a string
	name = (char*)luaL_checkstring(L, 1);

	// check if the second argument is a function
	if (!lua_isfunction(L, 2))
		return luaL_error(L, "second argument must be a function");

	// push the value of the second argument to the registry
	lua_pushvalue(L, 2);
	spref = luaL_ref(L, LUA_REGISTRYINDEX);

	// get the third argument
	logical = luaL_checkinteger(L, 3);
	
	// find if the function has already been registered
	for (i = 0; i < MAX_LUA_ENTS; ++i)
	{		
		if (st_lua_ents[i].function && st_lua_ents[i].function == spref)
		{
			lua_pushinteger(L, spref);
			return 1;
		}
	}

	// add new entry to st_lua_ents
	for (i = 0; i < MAX_LUA_ENTS; ++i)
	{
		// find the next empty slot
		if (!st_lua_ents[i].function)
		{
			st_lua_ents[i].name = G_NewString(name);
			st_lua_ents[i].function = spref;
			st_lua_ents[i].logical = logical;
			lua_pushinteger(L, spref);
			return 1;
		}
	}

	lua_pushinteger(L, spref);
	return 1;
}

//
// GEntity:Number( )
//
static int g_lua_GEntity_Number(lua_State *L)
{
	lua_GEntity *lent;

	lent = lua_getgentity(L, 1);

	lua_pushinteger(L, lent->e->s.number);
	return 1;
}

//
// GEntity:Position( )
// GEntity:Position( newVal:QVector )
//
static int g_lua_GEntity_Position(lua_State *L)
{
	int n = lua_gettop(L);
	lua_GEntity *lent;

	lent = lua_getgentity(L, 1);

	if (n > 1)
	{
		vec_t *newVal = lua_getvector(L, 2);
		VectorCopy(newVal, lent->e->r.currentOrigin);
		return 0;
	}

	lua_pushvector(L, lent->e->r.currentOrigin);
	return 1;
}

//
// GEntity:Angles( )
// GEntity:Angles( newVal:QVector )
//
static int g_lua_GEntity_Angles(lua_State *L)
{
	int n = lua_gettop(L);
	lua_GEntity *lent;

	lent = lua_getgentity(L, 1);

	if (n > 1)
	{
		vec_t *newVal = lua_getvector(L, 2);
		VectorCopy(newVal, lent->e->s.angles);
		return 0;
	}

	lua_pushvector(L, lent->e->s.angles);
	return 1;
}

//
// GEntity:Model( )
// GEntity:Model( newVal:String )
//
qboolean SpawnEntModel(gentity_t *ent, qboolean isSolid, qboolean isAnimated);
static int g_lua_GEntity_Model(lua_State *L)
{
	int n = lua_gettop(L);
	lua_GEntity *lent;

	lent = lua_getgentity(L, 1);

	if (n > 1)
	{
		lent->e->s.eFlags &= ~(EF_NODRAW);
		lent->e->s.modelGhoul2 = 0;
		lent->e->s.pos.trType = TR_STATIONARY;
		char *newVal = (char*)luaL_checkstring(L, 2);
		lent->e->model = G_NewString(newVal);
		G_SetOrigin( lent->e, lent->e->s.origin );
		VectorCopy(lent->e->s.origin, lent->e->s.pos.trBase );
		VectorCopy( lent->e->s.angles, lent->e->s.apos.trBase );
		SpawnEntModel(lent->e,qtrue,qfalse);
		trap_LinkEntity(lent->e);
		return 0;
	}

	lua_pushstring(L, lent->e->model);
	return 1;
}

//
// GEntity:Health( )
// GEntity:Health( newVal:Integer )
//
static int g_lua_GEntity_Health(lua_State *L)
{
	int n = lua_gettop(L);
	lua_GEntity *lent;

	lent = lua_getgentity(L, 1);

	if (n > 1)
	{		
		int newVal = luaL_checkinteger(L, 2);
		if (newVal>0)
			lent->e->takedamage = qtrue;
		lent->e->health = newVal;
		return 0;
	}

	lua_pushinteger(L, lent->e->health);
	return 1;
}

//
// GEntity:cliAimOrigin()
// GEntity:cliAimOrigin(range:Integer)
//
static int lua_GEntity_cliAimOrigin(lua_State *L)
{
	trace_t tr;
	lua_GEntity *lent = NULL;
	vec3_t start, end, forward;

	lent = lua_getcligentity(L,1);
	AngleVectors( lent->e->client->ps.viewangles, forward, NULL, NULL );
	VectorSet( start, lent->e->client->ps.origin[0], lent->e->client->ps.origin[1], lent->e->client->ps.origin[2] + lent->e->client->ps.viewheight );
	VectorMA( start, luaL_optinteger(L, 2, 131072), forward, end );

	trap_Trace(&tr, start, NULL, NULL, end, lent->e->s.number, MASK_SHOT);

	lua_pushvector(L, tr.endpos);
	return 1;
}

//
// GEntity:cliViewAngles( )
// GEntity:cliViewAngles( newVal:QVector )
//
static int g_lua_GEntity_cliViewAngles(lua_State *L)
{
	int n = lua_gettop(L);
	lua_GEntity *lent = lua_getcligentity(L, 1);

	if (n > 1)
	{
		vec_t *newVal = lua_getvector(L, 2);
		VectorCopy(newVal, lent->e->client->ps.viewangles);
		return 0;
	}

	lua_pushvector(L, lent->e->client->ps.viewangles);
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

//
// GEntity:cliWeapon( )
//
static int lua_GEntity_cliWeapon(lua_State *L)
{
	int n = lua_gettop(L);
	lua_GEntity *lent = lua_getcligentity(L, 1);

	lua_pushinteger(L, lent->e->client->ps.weapon);
	return 1;
}

//
// GEntity:cliPrintConsole( message:String )
//
static int lua_GEntity_cliPrintConsole(lua_State *L)
{
	int             i;
	char            buf[1000] = { 0 };
	int             n = lua_gettop(L);
	lua_GEntity		*lent = lua_getcligentity(L, 1);

	lua_rawgeti(L, LUA_REGISTRYINDEX, lua_toString);

	for (i = 2; i <= n; i++)
	{
		const char *s;

		lua_pushvalue(L, -1);	// function to be called
		lua_pushvalue(L, i);	// value to print
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);	// get result

		if (s == NULL)
			return luaL_error(L, "`tostring' must return a string to `print'");

		Q_strcat(buf, sizeof(buf), s);	
		lua_pop(L, 1);			// pop result
	}
	trap_SendServerCommand(lent->e->client->ps.clientNum, va("print \"%s\"", buf));
	return 0;
}

//
// GEntity:BindPain( callback:Function )
//
void lua_pain(gentity_t *self, gentity_t *attacker, int damage)
{
	lua_rawgeti(g_lua, LUA_REGISTRYINDEX, self->lua_pain);
	lua_pushgentity(g_lua, self);
	lua_pushgentity(g_lua, attacker);
	lua_pushinteger(g_lua, damage);

	if (lua_pcall(g_lua, 3, 0, 0))
		g_lua_reportError();
}

static int lua_GEntity_BindPain(lua_State *L)
{
	int n = lua_gettop(L);
	lua_GEntity *lent;

	if (n < 2)
		return luaL_error(L, "syntax: BindPain( callback:Function )");

	lent = lua_getgentity(L, 1);

	if (lua_isnil(L, 2))
	{
		if (lent->e->lua_pain > 0)
			luaL_unref(L, LUA_REGISTRYINDEX, lent->e->lua_pain);
		lent->e->lua_pain = 0;
	}
	else
	{
		lua_pushvalue(L, 2);
		lent->e->lua_pain = luaL_ref(L, LUA_REGISTRYINDEX);
		lent->e->pain = lua_pain;
	}
	return 0;
}

static const luaL_Reg gentity_ctor[] = {
	{ "FromNumber", g_lua_GEntity_FromNumber },
	{ "Place", g_lua_GEntity_Place},
	{ "Register", g_lua_GEntity_Register },
	
	{ NULL, NULL }
};

static const luaL_Reg gentity_meta[] = {
	{ "__gc", g_lua_GEntity_GC },

	{ "BindPain", lua_GEntity_BindPain},
	
	{ "cliAimOrigin", lua_GEntity_cliAimOrigin },
	{ "cliViewAngles", g_lua_GEntity_cliViewAngles },
	{ "cliName", lua_GEntity_cliName },
	{ "cliWeapon", lua_GEntity_cliWeapon },
	
	{ "cliPrintConsole", lua_GEntity_cliPrintConsole },
	
	{ "Number", g_lua_GEntity_Number },
	{ "Position", g_lua_GEntity_Position },
	{ "Angles", g_lua_GEntity_Angles},
	{ "Model", g_lua_GEntity_Model},
	{ "Health", g_lua_GEntity_Health},

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