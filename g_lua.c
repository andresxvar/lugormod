extern "C"{
    #include "lua/lua.h"
    #include "lua/lualib.h"
    #include "lua/lauxlib.h"
}
#include "g_lua.h"
#include "g_local.h"
#include "Lmd_Entities_Public.h"

lua_State *g_lua;
int lua_toString;

st_lua_cmd_t st_lua_cmds[MAX_LUA_CMDS];

/*
* Lua error reporting
*/
void g_lua_reportError()
{
	char errorMsg[512];

	Q_strncpyz(errorMsg, lua_tostring(g_lua, -1), sizeof(errorMsg));
	luaL_where(g_lua, 1);
	G_Printf(va("lua error: %s (%s)\n", errorMsg, lua_tostring(g_lua, -1)));
	lua_pop(g_lua, 2);
}

/*
* Lua script loading
*/
int g_lua_loadScript(char *fileName)
{
	fileHandle_t f;
	char fileBuffer[32000];
	int len;

	trap_Printf(va(" > loading %s...\n", fileName));

	len = trap_FS_FOpenFile(fileName, &f, FS_READ);
	if (!f || len >= 32000) {
		return 0;
	}

	trap_FS_Read(fileBuffer, len, f);
	fileBuffer[len] = 0;
	trap_FS_FCloseFile(f);

	if (luaL_loadbuffer(g_lua, fileBuffer, strlen(fileBuffer), fileName))
	{
		g_lua_reportError();
		return 0;
	}

	if (lua_pcall(g_lua, 0, 0, 0))
	{
		g_lua_reportError();
		return 0;
	}

	return 1;
}

void g_lua_init()
{
    G_Printf("-------- Lua Initialization ---------\n");
    G_Printf("- creating lua instance...\n");

    int i;
    int numGlobalScripts, globalScriptlen;
	char lstGlobalScripts[2048], *globalScriptPtr;
	
    // intialize commands
    for (i = 0; i < MAX_LUA_CMDS; i++)
	{
		memset(&st_lua_cmds[i], 0, sizeof(st_lua_cmd_t));
		st_lua_cmds[i].name = 0;
		st_lua_cmds[i].function = 0;
	}

    // initialize lua
    g_lua = luaL_newstate();

    // initialize librarys
    luaL_openlibs(g_lua);
    luaopen_game(g_lua);
    luaopen_gentity(g_lua);
	luaopen_vector(g_lua);


    numGlobalScripts = trap_FS_GetFileList("luascripts", ".lua", lstGlobalScripts, sizeof(lstGlobalScripts));
	globalScriptPtr = lstGlobalScripts;
	for (i = 0; i < numGlobalScripts; i++, globalScriptPtr += globalScriptlen + 1)
	{
		char filename[MAX_QPATH];
		globalScriptlen = strlen(globalScriptPtr);
		strcpy(filename, "luascripts/");
		strcat(filename, globalScriptPtr);
		g_lua_loadScript(filename);
	}    

    lua_getglobal(g_lua, "tostring");
	lua_toString = luaL_ref(g_lua, LUA_REGISTRYINDEX);    
}

void g_lua_shutdown()
{
	lua_close(g_lua);
}

int g_lua_callClCommand(gentity_t *ent, const char *cmd)
{
	for (int i = 0; i < MAX_LUA_CMDS; ++i)
	{
		if (st_lua_cmds[i].function && !Q_stricmp(st_lua_cmds[i].name, cmd))
		{
			lua_rawgeti(g_lua, LUA_REGISTRYINDEX, st_lua_cmds[i].function);
			lua_pushgentity(g_lua, ent);
			lua_pushinteger(g_lua, trap_Argc());

			if (lua_pcall(g_lua, 2, 1, 0))
			{
				g_lua_reportError();
				return 0;
			}
			else
			{
				int res = (int)lua_tonumber(g_lua, -1);
				return res;
			}
		}
	}
	return 0;
}

int g_lua_clientCommand(int clientId)
{	
	gentity_t *ent = g_entities + clientId;
	char	cmd[MAX_TOKEN_CHARS] = { 0 };

	if (ent->client && ent->client->pers.connected == CON_CONNECTED)
	{
		trap_Argv(0, cmd, sizeof(cmd));
		if (g_lua_callClCommand(ent, cmd))
			return 1;
	}
	return 0;
}