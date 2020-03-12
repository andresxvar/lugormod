Compilation Guide
-----------------

Linux:
(1) Download and install vscode
(2) open the folder that contains the code
(3) run make in the terminal
This will compile jampgamex86.so for 32bit targets (since the original lugormod is only 32bit)
Ignore the many warnings etc.


Note on dependent libraries: compiling requires the liblua.a (lua static library compiled for 32bit for compatibility).
You can make liblua.a bu running make command in the lua folder.
To compile liblua.a as 32bit you need to have the readline library as 32bit.
If your computer is 64bit machine, you can get readline library 32 bit by using:
sudo dpkg --add-architecture i386
sudo apt-get install libreadline-dev:i386

Optional Note: some recommended vscode plugins: c/c++ by Microsoft, Makefile by Markovic, Code Runner by J. Han.

Installation Guide
------------------

Linux:
If you don't want to compile, the jampgamex86.so is included in the "build" folder
(1) Create a directory in gamedata folder ex. "lmdx"
(2) Move jampgamex86.so to lmdx folder
(3) Create a directory for the scripts "lmdx/luascripts/"
(4) Add lua scripts ex. "lmdx/luascripts/rpg.lua"
(5) add a server.cfg (lmdx/server.cfg) and start a server ex:
./openjkded.i386 +set dedicated 2 +set net_port 29071 +set fs_game lmdx +exec server.cfg

Lua Features
------------
List of library and meta methods for game, entity and vector

// Game library methods
library: lua ex. Game.BindCommand("class", cmd_class_f)
	{ "BindCommand", 	g_lua_Game_BindCommand },
    { "Broadcast", 		g_lua_Game_Broadcast },
	{ "Argument", 		g_lua_Game_Argument },
	{ "ConcatArgs",		g_lua_Game_ConcatArgs },
	{ "PlayEffect", 	g_lua_Game_PlayEffect },

// Player library methods
static const luaL_Reg player_ctor[] = {
    { "FromEntity", g_lua_Player_FromEntity },
	{ "FromNumber", g_lua_Player_FromNumber },
		
	{ NULL, NULL }
};

// Player meta methods
static const luaL_Reg player_meta[] = {
	{ "__gc", g_lua_Player_GC },
	
	{ "Name", g_lua_Player_Name },
	{ "Number", g_lua_Player_Number },
	{ "Weapon", g_lua_Player_Weapon},
	{ "Hack", g_lua_Player_Hack},
	
	{ "AimOrigin", g_lua_Player_AimOrigin},
	{ "ViewAngles", g_lua_Player_ViewAngles },
    
	{ "PrintConsole", g_lua_Player_PrintConsole },
	{ "SiegeSpecTimer", g_lua_Player_SiegeSpecTimer },
	
	{ NULL, NULL }
};

// entity library methods
static const luaL_Reg gentity_ctor[] = {
	{ "FromNumber", g_lua_GEntity_FromNumber },
	{ "Place", g_lua_GEntity_Place},
	{ "Register", g_lua_GEntity_Register },

	{ "ReadSpawnVarInt", g_lua_GEntity_ReadSpawnVarInt},
	
	{ NULL, NULL }
};

// entity meta methods
static const luaL_Reg gentity_meta[] = {
	{ "__gc", g_lua_GEntity_GC },

	{ "Free", g_lua_GEntity_Free},
	{ "Blowup", g_lua_GEntity_Blowup},

	{ "BindPain", g_lua_GEntity_BindPain},
	//{ "BindTouch", lua_GEntity_BindTouch},
	//{ "BindTouch", lua_GEntity_BindDie},
	{ "BindUse", g_lua_GEntity_BindUse},
	{ "MakeHackable", g_lua_GEntity_MakeHackable},
		
	{ "Number", g_lua_GEntity_Number },
	{ "Angles", g_lua_GEntity_Angles},
	{ "Position", g_lua_GEntity_Position },
	
	{ "Model", g_lua_GEntity_Model},
	{ "Health", g_lua_GEntity_Health},
	{ "GenericValues", g_lua_GEntity_GenericValue},

	{ NULL, NULL }
};

- vector functions
static const luaL_Reg vector_lib[] = {
	{"New", Vector_New},
	{"Construct", Vector_Construct},
	{"Set", Vector_Set},
	{"Clear", Vector_Clear},
	{"Add", Vector_Add},
	{"Subtract", Vector_Subtract},
	{"Scale", Vector_Scale},
	{"Length", Vector_Length},
	{"Normalize", Vector_Normalize},
	{"NormalizeFast", Vector_NormalizeFast},
	//{"RotatePointAround", Vector_RotatePointAround},
	//{"Perpendicular", Vector_Perpendicular},
	{"Snap", Vector_Snap},
	{NULL, NULL}
};
static const luaL_Reg vector_meta[] = {
	{"__index", Vector_Index},
	{"__newindex", Vector_NewIndex},
	{"__add", Vector_AddOperator},
	{"__sub", Vector_SubOperator},
	{"__mul", Vector_DotOperator},
	{"__unm", Vector_NegateOperator},
	{"__gc", Vector_GC},
	{"__tostring", Vector_ToS1tring},
	{NULL, NULL}
};

Debuggin
--------
vscode: debugging on vscode is the best option
the launch configuration is included, you must modify the path for your application
run the build_install task
run the debugger

external terminal: Make sure the g++ was run with -g option
browse to openjkded.i386
run the gdb with arguments:
>> gdb --args ./openjkded.i386 +set dedicated 2 +set net_port 29071 +set fs_game lmdx +exec server.cfg
>> run
press ctrl+c go back to gdb
>> info sharedlibrary
note wether the symbols are loaded for jampgamei386.so
see that there is link to the source code:
>> info source
go back to openjkded
add breakpoint in hiscore function
>> break Lmd_Accounts.cpp:130
>> continue
make it break by running the server command hitime
>> info locals
