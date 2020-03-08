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
- Game.BindCommand(name:String, command:Function)
Can be used to make new commands for players, see rpg.lua example

- Game.Broadcast( message:String )
// Game.Broadcast( message:String, flags:Number )
// Game.Broadcast( message:String, flags:Number, playerId:Number )
Prints text to the chat, or screen, see rpg.lua example

...more features to come?