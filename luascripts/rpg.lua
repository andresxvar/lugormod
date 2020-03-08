function do_f(player,args)
	if args == 1 then
		player:cliPrintConsole("^7Syntax: /do <text>")
	else
		local msg = Game.ConcatArgs(1)
		Game.Broadcast("^7**^5" .. msg .. " ^7(" .. player:cliName() .. "^7)", 16)
	end
	return 1
end

function me_f(player,args)
	if args == 1 then
		player:cliPrintConsole("^7Syntax: /me <action>")
	else
		local msg = Game.ConcatArgs(1)
	Game.Broadcast("^2*^7" .. player:cliName() .. " ^7" .. msg, 16)
	end
	return 1
end

Game.BindCommand("do", do_f)
Game.BindCommand("me", me_f)