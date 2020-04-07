extern "C"
{
#include "lua/lua.h"
#include "lua/lauxlib.h"
}
#include "g_lua_main.h"

#include "g_local.h"
#include "Lmd_EntityCore.h"

static int g_lua_Player_GC(lua_State *L)
{
	//G_Printf("Lua says bye to Player = %p\n", lua_player(L));
	return 0;
}

//
// Player.FromNumber( id:Integer )
//
static int g_lua_Player_FromNumber(lua_State *L)
{
	int n = lua_gettop(L), num;

	if (n < 1)
		return luaL_error(L, "syntax: Player.FromNumber( id:Integer )");

	num = luaL_checkinteger(L, 1);

	if (num > level.maxclients)
		return luaL_error(L, va("number can't be more than %i", level.maxclients));

	g_lua_pushPlayer(L, &g_clients[num]);
	return 1;
}

//
// Player.FromEntity( ent:GEntity )
//
static int g_lua_Player_FromEntity(lua_State *L)
{
	int n = lua_gettop(L);
	lua_GEntity *lent;
	gclient_t *cl;
	if (n < 1)
		return luaL_error(L, "syntax: Player.FromEntity( ent:GEntity )");

	lent = g_lua_checkEntity(L, 1);
	cl = lent->e->client;

	if (!cl)
		return luaL_error(L, "entity is not a player");

	g_lua_pushPlayer(L, cl);
	return 1;
}

//
// Player:Name( )
// Player:Name( newVal:String )
//
static int g_lua_Player_Name(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	if (n > 1)
	{
		char *newVal = (char *)luaL_checkstring(L, 2);
		Q_strncpyz(ply->cl->pers.netname, newVal, sizeof(ply->cl->pers.netname));
		ply->cl->pers.netnameTime = 0;
		ClientUserinfoChanged(ply->cl->ps.clientNum);
		ply->cl->pers.netnameTime = 0;
		return 0;
	}
	else
	{
		lua_pushstring(L, ply->cl->pers.netname);
		return 1;
	}
}

//
// Player:Number( )
//
static int g_lua_Player_Number(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	lua_pushinteger(L, ply->cl->ps.clientNum);
	return 1;
}

//
// Player:MaxHealth( )
// Player:MaxHealth( newVal:Integer )
//
static int g_lua_Player_MaxHealth(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	if (n > 1)
	{
		int newVal = luaL_checkinteger(L, 2);
		gentity_t *ent = &g_entities[ply->cl->ps.clientNum];
		ply->cl->pers.maxHealth = ply->cl->ps.stats[STAT_MAX_HEALTH] = newVal;
		ent->maxHealth = ent->s.maxhealth = newVal;
		return 0;
	}

	lua_pushinteger(L, ply->cl->pers.maxHealth);
	return 1;
}

//
// Player:Health( )
// Player:Health( newVal:Integer )
//
static int g_lua_Player_Health(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);
	gentity_t *ent = &g_entities[ply->cl->ps.clientNum];

	if (n > 1)
	{
		int newVal = luaL_checkinteger(L, 2);
		ply->cl->ps.stats[STAT_HEALTH] = newVal;
		ent->health = ent->s.health = newVal;
		return 0;
	}

	lua_pushinteger(L, ent->health);
	return 1;
}

//
// GEntity:Position( )
// GEntity:Position( newVal:QVector )
//
static int g_lua_Player_Position(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	if (n > 1)
	{
		vec_t *newVal = lua_getvector(L, 2);
		VectorCopy(newVal, ply->cl->ps.origin);
		return 0;
	}

	lua_pushvector(L, ply->cl->ps.origin);
	return 1;
}

//
// Player:SiegeSpecTimer( waitTime:Integer )
//
static int g_lua_Player_SiegeSpecTimer(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	int waitTime = luaL_checkinteger(L, 2);

	gentity_t *te = G_TempEntity(ply->cl->ps.origin, EV_SIEGESPEC);
	te->s.time = level.time + waitTime * 1000;
	te->s.owner = ply->cl->ps.clientNum;
	return 0;
}

//
// Player:Weapon( )
//
static int g_lua_Player_Weapon(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	lua_pushinteger(L, ply->cl->ps.weapon);
	return 1;
}

//
// Player:AdminRank( )
//
int Auths_GetPlayerRank(gentity_t *ent);
static int g_lua_Player_AdminRank(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	lua_pushinteger(L, Auths_GetPlayerRank(&g_entities[ply->cl->ps.clientNum]));
	return 1;
}

//
// Player:Hack(ent:GEntity,time:Intenger)
//
static int g_lua_Player_Hack(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);
	lua_GEntity *ent = g_lua_checkEntity(L, 2);
	int hacktime = luaL_checkinteger(L, 3);

	if (ply->cl->isHacking != ent->e->genericValue10)
	{ // start hacking
		ply->cl->isHacking = ent->e->genericValue10;
		ply->cl->ps.hackingTime = level.time + hacktime;
		ply->cl->ps.hackingBaseTime = hacktime;
		VectorCopy(ply->cl->ps.viewangles, ply->cl->hackingAngles);
		if (ent->e->aimDebounceTime < level.time)
		{
			ent->e->aimDebounceTime = level.time + 3000;
		}
	}
	else if (ply->cl->ps.hackingTime < level.time)
	{
		// finished hacking
		ply->cl->isHacking = 0;
		ply->cl->ps.hackingTime = 0;
		ply->cl->ps.hackingBaseTime = 0;
		lua_pushboolean(L, qtrue);
		return 1;
	}

	// still hacking
	lua_pushboolean(L, qfalse);
	return 1;
}

//
// Player:NPCSpawn(npc_type:String, npc_targetname:String)
//
extern gentity_t *NPC_SpawnType(gentity_t *ent, char *npc_type, char *targetname, qboolean isVehicle);
static int g_lua_Player_NPCSpawn(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);
	gentity_t *ent = &g_entities[ply->cl->ps.clientNum];
	char *npc_type = (char *)luaL_checkstring(L, 2);
	char *npc_targetname = (char *)luaL_checkstring(L, 3);

	gentity_t *newEnt = NPC_SpawnType(ent, npc_type, npc_targetname, qfalse);

	if (!newEnt)
		lua_pushnil(L);
	else
		g_lua_pushEntity(L, newEnt);
	return 1;
}

//
// Player:Team()
// Player:Team(team:String,wait:Integer)
//
static int g_lua_Player_Team(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	if (n > 1)
	{
		char *newTeam = (char *)luaL_checkstring(L, 2);
		int switchTime = luaL_checkinteger(L, 3);
		SetTeam(&g_entities[ply->cl->ps.clientNum], newTeam);
		ply->cl->switchTeamTime = level.time + switchTime * 1000;
		return 0;
	}

	lua_pushinteger(L, ply->cl->sess.sessionTeam);
	return 1;
}

//
// Player:BoltModel(range:Integer)
//
void lua_BoltObject_Think(gentity_t *bolt)
{
	gentity_t *player = &g_entities[bolt->s.boltToPlayer-1];

	if (player->client->pers.connected == CON_CONNECTED) {
		VectorCopy(player->r.currentOrigin, bolt->r.currentOrigin);
		trap_LinkEntity(bolt);		
	}
	else
	{
		player->client->holdingObjectiveItem = 0;
		G_FreeEntity(bolt);
	}

	bolt->nextthink = level.time + 3000;
}

qboolean SpawnEntModel(gentity_t *ent, qboolean isSolid, qboolean isAnimated);
static int g_lua_Player_BoltModel(lua_State *L)
{
	lua_Player *ply = g_lua_checkPlayer(L, 1);
	char *model = (char *)luaL_checkstring(L, 2);
	float modelscale = luaL_checknumber(L, 3);
	gentity_t *plyEnt = &g_entities[ply->cl->ps.clientNum];
	
	if (ply->cl->holdingObjectiveItem) // remove the old bolt object
	{
		gentity_t *oldBolt = &g_entities[ply->cl->holdingObjectiveItem];
		if (oldBolt && oldBolt->inuse)
		{
			ply->cl->holdingObjectiveItem = 0;
			G_FreeEntity(oldBolt);
		}
	}

	// attach a new bolt object
	gentity_t *bolt = G_Spawn();
	if (bolt)
	{
		bolt->model = model;
		bolt->modelScale[0] = modelscale;
		SpawnEntModel(bolt,qfalse,false);
		G_SetOrigin(bolt, plyEnt->r.currentOrigin);
		bolt->s.boltToPlayer = ply->cl->ps.clientNum + 1;
		bolt->think = lua_BoltObject_Think;
		bolt->nextthink = level.time + FRAMETIME;
		ply->cl->holdingObjectiveItem = bolt->s.number;		
		trap_LinkEntity(bolt);
	}
	return 0;
}

//
// Player:AimOrigin()
// Player:AimOrigin(range:Integer)
//
static int g_lua_Player_AimOrigin(lua_State *L)
{
	trace_t tr;
	lua_Player *ply = NULL;
	vec3_t start, end, forward;

	ply = g_lua_checkPlayer(L, 1);
	AngleVectors(ply->cl->ps.viewangles, forward, NULL, NULL);
	VectorSet(start, ply->cl->ps.origin[0], ply->cl->ps.origin[1], ply->cl->ps.origin[2] + ply->cl->ps.viewheight);
	VectorMA(start, luaL_optinteger(L, 2, 131072), forward, end);

	trap_Trace(&tr, start, NULL, NULL, end, ply->cl->ps.clientNum, MASK_SHOT);

	lua_pushvector(L, tr.endpos);
	return 1;
}

//
// Player:AimAnyTarget(range:Integer)
//
gentity_t *AimAnyTarget(gentity_t *ent, int length);
static int g_lua_Player_AimAnyTarget(lua_State *L)
{
	lua_Player *ply = g_lua_checkPlayer(L, 1);
	gentity_t *player = &g_entities[ply->cl->ps.clientNum];
	gentity_t *target = NULL;

	int range = luaL_checkinteger(L, 2);
	gentity_t *targetEnt = AimAnyTarget(player, range);

	if (!targetEnt)
		lua_pushnil(L);
	else
		g_lua_pushEntity(L, targetEnt);
	return 1;
}

//
// Player:ViewAngles( )
// Player:ViewAngles( newVal:QVector )
//
static int g_lua_Player_ViewAngles(lua_State *L)
{
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	if (n > 1)
	{
		vec_t *newVal = lua_getvector(L, 2);
		VectorCopy(newVal, ply->cl->ps.viewangles);
		return 0;
	}

	lua_pushvector(L, ply->cl->ps.viewangles);
	return 1;
}

//
// Player:PrintChat( message:String )
//
static int g_lua_Player_PrintChat(lua_State *L)
{
	int i;
	char buf[1000] = {0};
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	lua_rawgeti(L, LUA_REGISTRYINDEX, lua_toString);

	for (i = 2; i <= n; i++)
	{
		const char *s;

		lua_pushvalue(L, -1); // function to be called
		lua_pushvalue(L, i);  // value to print
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1); // get result

		if (s == NULL)
			return luaL_error(L, "`tostring' must return a string to `print'");

		Q_strcat(buf, sizeof(buf), s);
		lua_pop(L, 1); // pop result
	}
	trap_SendServerCommand(ply->cl->ps.clientNum, va("chat \"%s\"", buf));
	return 0;
}

//
// Player:PrintConsole( message:String )
//
static int g_lua_Player_PrintConsole(lua_State *L)
{
	int i;
	char buf[1000] = {0};
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	lua_rawgeti(L, LUA_REGISTRYINDEX, lua_toString);

	for (i = 2; i <= n; i++)
	{
		const char *s;

		lua_pushvalue(L, -1); // function to be called
		lua_pushvalue(L, i);  // value to print
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1); // get result

		if (s == NULL)
			return luaL_error(L, "`tostring' must return a string to `print'");

		Q_strcat(buf, sizeof(buf), s);
		lua_pop(L, 1); // pop result
	}
	trap_SendServerCommand(ply->cl->ps.clientNum, va("print \"%s\"", buf));
	return 0;
}

//
// Player:PrintCenter( message:String )
//
static int g_lua_Player_PrintCenter(lua_State *L)
{
	int i;
	char buf[1000] = {0};
	int n = lua_gettop(L);
	lua_Player *ply = g_lua_checkPlayer(L, 1);

	lua_rawgeti(L, LUA_REGISTRYINDEX, lua_toString);

	for (i = 2; i <= n; i++)
	{
		const char *s;

		lua_pushvalue(L, -1); // function to be called
		lua_pushvalue(L, i);  // value to print
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1); // get result

		if (s == NULL)
			return luaL_error(L, "`tostring' must return a string to `print'");

		Q_strcat(buf, sizeof(buf), s);
		lua_pop(L, 1); // pop result
	}
	trap_SendServerCommand(ply->cl->ps.clientNum, va("cp \"%s\"", buf));
	return 0;
}

// Player library methods
static const luaL_Reg player_ctor[] = {
	{"FromEntity", g_lua_Player_FromEntity},
	{"FromNumber", g_lua_Player_FromNumber},

	{NULL, NULL}};

// Player meta methods
static const luaL_Reg player_meta[] = {
	{"__gc", g_lua_Player_GC},

	{"Name", g_lua_Player_Name},
	{"Number", g_lua_Player_Number},
	{"Health", g_lua_Player_Health},
	{"MaxHealth", g_lua_Player_MaxHealth},
	{"Position", g_lua_Player_Position},
	{"Weapon", g_lua_Player_Weapon},
	{"Hack", g_lua_Player_Hack},
	{"NPCSpawn", g_lua_Player_NPCSpawn},
	{"Team", g_lua_Player_Team},

	{"AdminRank", g_lua_Player_AdminRank},

	{"BoltModel", g_lua_Player_BoltModel},
	{"AimOrigin", g_lua_Player_AimOrigin},
	{"AimAnyTarget", g_lua_Player_AimAnyTarget},
	{"ViewAngles", g_lua_Player_ViewAngles},

	{"PrintConsole", g_lua_Player_PrintConsole},
	{"PrintChat", g_lua_Player_PrintChat},
	{"PrintCenter", g_lua_Player_PrintCenter},
	{"SiegeSpecTimer", g_lua_Player_SiegeSpecTimer},

	{NULL, NULL}};

int luaopen_player(lua_State *L)
{
	// set Player object functions
	luaL_newlib(L, player_ctor);

	// set Player class metatable
	luaL_newmetatable(L, "Game.Player"); /* create metatable for entities */
	lua_pushvalue(L, -1);				 /* push metatable */
	lua_setfield(L, -2, "__index");		 /* metatable.__index = metatable */
	luaL_setfuncs(L, player_meta, 0);	 /* add entity methods to new metatable */
	lua_pop(L, 1);						 /* pop new metatable */

	// set global class
	lua_setglobal(L, "Player");

	return 1;
}

void g_lua_pushPlayer(lua_State *L, gclient_t *cl)
{
	lua_Player *ply;

	ply = (lua_Player *)lua_newuserdata(L, sizeof(lua_Player));

	luaL_getmetatable(L, "Game.Player");
	lua_setmetatable(L, -2);

	ply->cl = cl;
}

lua_Player *g_lua_checkPlayer(lua_State *L, int argNum)
{
	void *ud;
	lua_Player *ply;

	ud = luaL_checkudata(L, argNum, "Game.Player");
	luaL_argcheck(L, ud != NULL, argNum, "`player' expected");

	ply = (lua_Player *)ud;
	return ply;
}