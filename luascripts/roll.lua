function throw(player,args)
	Game.Broadcast("^2*^7" .. player:cliName() .. " ^7 rolled a ^1" .. math.random(6), 16)
	return 1
end

Game.BindCommand("roll", throw)
